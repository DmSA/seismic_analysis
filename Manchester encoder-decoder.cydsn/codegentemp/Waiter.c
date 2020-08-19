/*******************************************************************************
* File Name: Waiter.c  
* Version 3.0
*
*  Description:
*     The Counter component consists of a 8, 16, 24 or 32-bit counter with
*     a selectable period between 2 and 2^Width - 1.  
*
*   Note:
*     None
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "Waiter.h"

uint8 Waiter_initVar = 0u;


/*******************************************************************************
* Function Name: Waiter_Init
********************************************************************************
* Summary:
*     Initialize to the schematic state
* 
* Parameters:  
*  void  
*
* Return: 
*  void
*
*******************************************************************************/
void Waiter_Init(void) 
{
        #if (!Waiter_UsingFixedFunction && !Waiter_ControlRegRemoved)
            uint8 ctrl;
        #endif /* (!Waiter_UsingFixedFunction && !Waiter_ControlRegRemoved) */
        
        #if(!Waiter_UsingFixedFunction) 
            /* Interrupt State Backup for Critical Region*/
            uint8 Waiter_interruptState;
        #endif /* (!Waiter_UsingFixedFunction) */
        
        #if (Waiter_UsingFixedFunction)
            /* Clear all bits but the enable bit (if it's already set for Timer operation */
            Waiter_CONTROL &= Waiter_CTRL_ENABLE;
            
            /* Clear the mode bits for continuous run mode */
            #if (CY_PSOC5A)
                Waiter_CONTROL2 &= ((uint8)(~Waiter_CTRL_MODE_MASK));
            #endif /* (CY_PSOC5A) */
            #if (CY_PSOC3 || CY_PSOC5LP)
                Waiter_CONTROL3 &= ((uint8)(~Waiter_CTRL_MODE_MASK));                
            #endif /* (CY_PSOC3 || CY_PSOC5LP) */
            /* Check if One Shot mode is enabled i.e. RunMode !=0*/
            #if (Waiter_RunModeUsed != 0x0u)
                /* Set 3rd bit of Control register to enable one shot mode */
                Waiter_CONTROL |= Waiter_ONESHOT;
            #endif /* (Waiter_RunModeUsed != 0x0u) */
            
            /* Set the IRQ to use the status register interrupts */
            Waiter_CONTROL2 |= Waiter_CTRL2_IRQ_SEL;
            
            /* Clear and Set SYNCTC and SYNCCMP bits of RT1 register */
            Waiter_RT1 &= ((uint8)(~Waiter_RT1_MASK));
            Waiter_RT1 |= Waiter_SYNC;     
                    
            /*Enable DSI Sync all all inputs of the Timer*/
            Waiter_RT1 &= ((uint8)(~Waiter_SYNCDSI_MASK));
            Waiter_RT1 |= Waiter_SYNCDSI_EN;

        #else
            #if(!Waiter_ControlRegRemoved)
            /* Set the default compare mode defined in the parameter */
            ctrl = Waiter_CONTROL & ((uint8)(~Waiter_CTRL_CMPMODE_MASK));
            Waiter_CONTROL = ctrl | Waiter_DEFAULT_COMPARE_MODE;
            
            /* Set the default capture mode defined in the parameter */
            ctrl = Waiter_CONTROL & ((uint8)(~Waiter_CTRL_CAPMODE_MASK));
            
            #if( 0 != Waiter_CAPTURE_MODE_CONF)
                Waiter_CONTROL = ctrl | Waiter_DEFAULT_CAPTURE_MODE;
            #else
                Waiter_CONTROL = ctrl;
            #endif /* 0 != Waiter_CAPTURE_MODE */ 
            
            #endif /* (!Waiter_ControlRegRemoved) */
        #endif /* (Waiter_UsingFixedFunction) */
        
        /* Clear all data in the FIFO's */
        #if (!Waiter_UsingFixedFunction)
            Waiter_ClearFIFO();
        #endif /* (!Waiter_UsingFixedFunction) */
        
        /* Set Initial values from Configuration */
        Waiter_WritePeriod(Waiter_INIT_PERIOD_VALUE);
        #if (!(Waiter_UsingFixedFunction && (CY_PSOC5A)))
            Waiter_WriteCounter(Waiter_INIT_COUNTER_VALUE);
        #endif /* (!(Waiter_UsingFixedFunction && (CY_PSOC5A))) */
        Waiter_SetInterruptMode(Waiter_INIT_INTERRUPTS_MASK);
        
        #if (!Waiter_UsingFixedFunction)
            /* Read the status register to clear the unwanted interrupts */
            (void)Waiter_ReadStatusRegister();
            /* Set the compare value (only available to non-fixed function implementation */
            Waiter_WriteCompare(Waiter_INIT_COMPARE_VALUE);
            /* Use the interrupt output of the status register for IRQ output */
            
            /* CyEnterCriticalRegion and CyExitCriticalRegion are used to mark following region critical*/
            /* Enter Critical Region*/
            Waiter_interruptState = CyEnterCriticalSection();
            
            Waiter_STATUS_AUX_CTRL |= Waiter_STATUS_ACTL_INT_EN_MASK;
            
            /* Exit Critical Region*/
            CyExitCriticalSection(Waiter_interruptState);
            
        #endif /* (!Waiter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: Waiter_Enable
********************************************************************************
* Summary:
*     Enable the Counter
* 
* Parameters:  
*  void  
*
* Return: 
*  void
*
* Side Effects: 
*   If the Enable mode is set to Hardware only then this function has no effect 
*   on the operation of the counter.
*
*******************************************************************************/
void Waiter_Enable(void) 
{
    /* Globally Enable the Fixed Function Block chosen */
    #if (Waiter_UsingFixedFunction)
        Waiter_GLOBAL_ENABLE |= Waiter_BLOCK_EN_MASK;
        Waiter_GLOBAL_STBY_ENABLE |= Waiter_BLOCK_STBY_EN_MASK;
    #endif /* (Waiter_UsingFixedFunction) */  
        
    /* Enable the counter from the control register  */
    /* If Fixed Function then make sure Mode is set correctly */
    /* else make sure reset is clear */
    #if(!Waiter_ControlRegRemoved || Waiter_UsingFixedFunction)
        Waiter_CONTROL |= Waiter_CTRL_ENABLE;                
    #endif /* (!Waiter_ControlRegRemoved || Waiter_UsingFixedFunction) */
    
}


/*******************************************************************************
* Function Name: Waiter_Start
********************************************************************************
* Summary:
*  Enables the counter for operation 
*
* Parameters:  
*  void  
*
* Return: 
*  void
*
* Global variables:
*  Waiter_initVar: Is modified when this function is called for the  
*   first time. Is used to ensure that initialization happens only once.
*
*******************************************************************************/
void Waiter_Start(void) 
{
    if(Waiter_initVar == 0u)
    {
        Waiter_Init();
        
        Waiter_initVar = 1u; /* Clear this bit for Initialization */        
    }
    
    /* Enable the Counter */
    Waiter_Enable();        
}


/*******************************************************************************
* Function Name: Waiter_Stop
********************************************************************************
* Summary:
* Halts the counter, but does not change any modes or disable interrupts.
*
* Parameters:  
*  void  
*
* Return: 
*  void
*
* Side Effects: If the Enable mode is set to Hardware only then this function
*               has no effect on the operation of the counter.
*
*******************************************************************************/
void Waiter_Stop(void) 
{
    /* Disable Counter */
    #if(!Waiter_ControlRegRemoved || Waiter_UsingFixedFunction)
        Waiter_CONTROL &= ((uint8)(~Waiter_CTRL_ENABLE));        
    #endif /* (!Waiter_ControlRegRemoved || Waiter_UsingFixedFunction) */
    
    /* Globally disable the Fixed Function Block chosen */
    #if (Waiter_UsingFixedFunction)
        Waiter_GLOBAL_ENABLE &= ((uint8)(~Waiter_BLOCK_EN_MASK));
        Waiter_GLOBAL_STBY_ENABLE &= ((uint8)(~Waiter_BLOCK_STBY_EN_MASK));
    #endif /* (Waiter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: Waiter_SetInterruptMode
********************************************************************************
* Summary:
* Configures which interrupt sources are enabled to generate the final interrupt
*
* Parameters:  
*  InterruptsMask: This parameter is an or'd collection of the status bits
*                   which will be allowed to generate the counters interrupt.   
*
* Return: 
*  void
*
*******************************************************************************/
void Waiter_SetInterruptMode(uint8 interruptsMask) 
{
    Waiter_STATUS_MASK = interruptsMask;
}


/*******************************************************************************
* Function Name: Waiter_ReadStatusRegister
********************************************************************************
* Summary:
*   Reads the status register and returns it's state. This function should use
*       defined types for the bit-field information as the bits in this
*       register may be permuteable.
*
* Parameters:  
*  void
*
* Return: 
*  (uint8) The contents of the status register
*
* Side Effects:
*   Status register bits may be clear on read. 
*
*******************************************************************************/
uint8   Waiter_ReadStatusRegister(void) 
{
    return Waiter_STATUS;
}


#if(!Waiter_ControlRegRemoved)
/*******************************************************************************
* Function Name: Waiter_ReadControlRegister
********************************************************************************
* Summary:
*   Reads the control register and returns it's state. This function should use
*       defined types for the bit-field information as the bits in this
*       register may be permuteable.
*
* Parameters:  
*  void
*
* Return: 
*  (uint8) The contents of the control register
*
*******************************************************************************/
uint8   Waiter_ReadControlRegister(void) 
{
    return Waiter_CONTROL;
}


/*******************************************************************************
* Function Name: Waiter_WriteControlRegister
********************************************************************************
* Summary:
*   Sets the bit-field of the control register.  This function should use
*       defined types for the bit-field information as the bits in this
*       register may be permuteable.
*
* Parameters:  
*  void
*
* Return: 
*  (uint8) The contents of the control register
*
*******************************************************************************/
void    Waiter_WriteControlRegister(uint8 control) 
{
    Waiter_CONTROL = control;
}

#endif  /* (!Waiter_ControlRegRemoved) */


#if (!(Waiter_UsingFixedFunction && (CY_PSOC5A)))
/*******************************************************************************
* Function Name: Waiter_WriteCounter
********************************************************************************
* Summary:
*   This funtion is used to set the counter to a specific value
*
* Parameters:  
*  counter:  New counter value. 
*
* Return: 
*  void 
*
*******************************************************************************/
void Waiter_WriteCounter(uint8 counter) \
                                   
{
    #if(Waiter_UsingFixedFunction)
        /* assert if block is already enabled */
        CYASSERT (0u == (Waiter_GLOBAL_ENABLE & Waiter_BLOCK_EN_MASK));
        /* If block is disabled, enable it and then write the counter */
        Waiter_GLOBAL_ENABLE |= Waiter_BLOCK_EN_MASK;
        CY_SET_REG16(Waiter_COUNTER_LSB_PTR, (uint16)counter);
        Waiter_GLOBAL_ENABLE &= ((uint8)(~Waiter_BLOCK_EN_MASK));
    #else
        CY_SET_REG8(Waiter_COUNTER_LSB_PTR, counter);
    #endif /* (Waiter_UsingFixedFunction) */
}
#endif /* (!(Waiter_UsingFixedFunction && (CY_PSOC5A))) */


/*******************************************************************************
* Function Name: Waiter_ReadCounter
********************************************************************************
* Summary:
* Returns the current value of the counter.  It doesn't matter
* if the counter is enabled or running.
*
* Parameters:  
*  void:  
*
* Return: 
*  (uint8) The present value of the counter.
*
*******************************************************************************/
uint8 Waiter_ReadCounter(void) 
{
    /* Force capture by reading Accumulator */
    /* Must first do a software capture to be able to read the counter */
    /* It is up to the user code to make sure there isn't already captured data in the FIFO */
    #if(Waiter_UsingFixedFunction)
		(void)CY_GET_REG16(Waiter_COUNTER_LSB_PTR);
	#else
		(void)CY_GET_REG8(Waiter_COUNTER_LSB_PTR_8BIT);
	#endif/* (Waiter_UsingFixedFunction) */
    
    /* Read the data from the FIFO (or capture register for Fixed Function)*/
    #if(Waiter_UsingFixedFunction)
        return ((uint8)CY_GET_REG16(Waiter_STATICCOUNT_LSB_PTR));
    #else
        return (CY_GET_REG8(Waiter_STATICCOUNT_LSB_PTR));
    #endif /* (Waiter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: Waiter_ReadCapture
********************************************************************************
* Summary:
*   This function returns the last value captured.
*
* Parameters:  
*  void
*
* Return: 
*  (uint8) Present Capture value.
*
*******************************************************************************/
uint8 Waiter_ReadCapture(void) 
{
    #if(Waiter_UsingFixedFunction)
        return ((uint8)CY_GET_REG16(Waiter_STATICCOUNT_LSB_PTR));
    #else
        return (CY_GET_REG8(Waiter_STATICCOUNT_LSB_PTR));
    #endif /* (Waiter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: Waiter_WritePeriod
********************************************************************************
* Summary:
* Changes the period of the counter.  The new period 
* will be loaded the next time terminal count is detected.
*
* Parameters:  
*  period: (uint8) A value of 0 will result in
*         the counter remaining at zero.  
*
* Return: 
*  void
*
*******************************************************************************/
void Waiter_WritePeriod(uint8 period) 
{
    #if(Waiter_UsingFixedFunction)
        CY_SET_REG16(Waiter_PERIOD_LSB_PTR,(uint16)period);
    #else
        CY_SET_REG8(Waiter_PERIOD_LSB_PTR, period);
    #endif /* (Waiter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: Waiter_ReadPeriod
********************************************************************************
* Summary:
* Reads the current period value without affecting counter operation.
*
* Parameters:  
*  void:  
*
* Return: 
*  (uint8) Present period value.
*
*******************************************************************************/
uint8 Waiter_ReadPeriod(void) 
{
    #if(Waiter_UsingFixedFunction)
        return ((uint8)CY_GET_REG16(Waiter_PERIOD_LSB_PTR));
    #else
        return (CY_GET_REG8(Waiter_PERIOD_LSB_PTR));
    #endif /* (Waiter_UsingFixedFunction) */
}


#if (!Waiter_UsingFixedFunction)
/*******************************************************************************
* Function Name: Waiter_WriteCompare
********************************************************************************
* Summary:
* Changes the compare value.  The compare output will 
* reflect the new value on the next UDB clock.  The compare output will be 
* driven high when the present counter value compares true based on the 
* configured compare mode setting. 
*
* Parameters:  
*  Compare:  New compare value. 
*
* Return: 
*  void
*
*******************************************************************************/
void Waiter_WriteCompare(uint8 compare) \
                                   
{
    #if(Waiter_UsingFixedFunction)
        CY_SET_REG16(Waiter_COMPARE_LSB_PTR, (uint16)compare);
    #else
        CY_SET_REG8(Waiter_COMPARE_LSB_PTR, compare);
    #endif /* (Waiter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: Waiter_ReadCompare
********************************************************************************
* Summary:
* Returns the compare value.
*
* Parameters:  
*  void:
*
* Return: 
*  (uint8) Present compare value.
*
*******************************************************************************/
uint8 Waiter_ReadCompare(void) 
{
    return (CY_GET_REG8(Waiter_COMPARE_LSB_PTR));
}


#if (Waiter_COMPARE_MODE_SOFTWARE)
/*******************************************************************************
* Function Name: Waiter_SetCompareMode
********************************************************************************
* Summary:
*  Sets the software controlled Compare Mode.
*
* Parameters:
*  compareMode:  Compare Mode Enumerated Type.
*
* Return:
*  void
*
*******************************************************************************/
void Waiter_SetCompareMode(uint8 compareMode) 
{
    /* Clear the compare mode bits in the control register */
    Waiter_CONTROL &= ((uint8)(~Waiter_CTRL_CMPMODE_MASK));
    
    /* Write the new setting */
    Waiter_CONTROL |= compareMode;
}
#endif  /* (Waiter_COMPARE_MODE_SOFTWARE) */


#if (Waiter_CAPTURE_MODE_SOFTWARE)
/*******************************************************************************
* Function Name: Waiter_SetCaptureMode
********************************************************************************
* Summary:
*  Sets the software controlled Capture Mode.
*
* Parameters:
*  captureMode:  Capture Mode Enumerated Type.
*
* Return:
*  void
*
*******************************************************************************/
void Waiter_SetCaptureMode(uint8 captureMode) 
{
    /* Clear the capture mode bits in the control register */
    Waiter_CONTROL &= ((uint8)(~Waiter_CTRL_CAPMODE_MASK));
    
    /* Write the new setting */
    Waiter_CONTROL |= ((uint8)((uint8)captureMode << Waiter_CTRL_CAPMODE0_SHIFT));
}
#endif  /* (Waiter_CAPTURE_MODE_SOFTWARE) */


/*******************************************************************************
* Function Name: Waiter_ClearFIFO
********************************************************************************
* Summary:
*   This function clears all capture data from the capture FIFO
*
* Parameters:  
*  void:
*
* Return: 
*  None
*
*******************************************************************************/
void Waiter_ClearFIFO(void) 
{

    while(0u != (Waiter_ReadStatusRegister() & Waiter_STATUS_FIFONEMP))
    {
        (void)Waiter_ReadCapture();
    }

}
#endif  /* (!Waiter_UsingFixedFunction) */


/* [] END OF FILE */

