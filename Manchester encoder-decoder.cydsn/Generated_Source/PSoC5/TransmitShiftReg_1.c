/*******************************************************************************
* File Name: TransmitShiftReg_1.c
* Version 2.30
*
* Description:
*  This file provides the API source code for the Shift Register component.
*
* Note: none
*
********************************************************************************
* Copyright 2008-2013, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
********************************************************************************/

#include "TransmitShiftReg_1.h"

uint8 TransmitShiftReg_1_initVar = 0u;


/*******************************************************************************
* Function Name: TransmitShiftReg_1_Start
********************************************************************************
*
* Summary:
*  Starts the Shift Register.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  TransmitShiftReg_1_initVar - used to check initial configuration, modified on
*  first function call.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void TransmitShiftReg_1_Start(void) 
{
    if(0u == TransmitShiftReg_1_initVar)
    {
        TransmitShiftReg_1_Init();
        TransmitShiftReg_1_initVar = 1u; /* Component initialized */
    }

    TransmitShiftReg_1_Enable();
}


/*******************************************************************************
* Function Name: TransmitShiftReg_1_Enable
********************************************************************************
*
* Summary:
*  Enables the Shift Register.
*
* Parameters:
*  void.
*
* Return:
*  void.
*
*******************************************************************************/
void TransmitShiftReg_1_Enable(void) 
{
    /* Changing address in Datapath Control Store
       from NOP to component state machine commands space */
    TransmitShiftReg_1_SR_CONTROL |= TransmitShiftReg_1_CLK_EN;

    TransmitShiftReg_1_EnableInt();
}


/*******************************************************************************
* Function Name: TransmitShiftReg_1_Init
********************************************************************************
*
* Summary:
*  Initializes Tx and/or Rx interrupt sources with initial values.
*
* Parameters:
*  void.
*
* Return:
*  void.
*
*******************************************************************************/
void TransmitShiftReg_1_Init(void) 
{
    TransmitShiftReg_1_SetIntMode(TransmitShiftReg_1_INT_SRC);
}


/*******************************************************************************
* Function Name: TransmitShiftReg_1_Stop
********************************************************************************
*
* Summary:
*  Disables the Shift Register
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void TransmitShiftReg_1_Stop(void) 
{
    /*changing Datapath Control Store address to NOP space*/
    TransmitShiftReg_1_SR_CONTROL &= ((uint8) ~TransmitShiftReg_1_CLK_EN);
    TransmitShiftReg_1_DisableInt();
}


/*******************************************************************************
* Function Name: TransmitShiftReg_1_EnableInt
********************************************************************************
*
* Summary:
*  Enables the Shift Register interrupt.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void TransmitShiftReg_1_EnableInt(void) 
{
    uint8 interruptState;

    interruptState = CyEnterCriticalSection();
    TransmitShiftReg_1_SR_AUX_CONTROL |= TransmitShiftReg_1_INTERRUPTS_ENABLE;
    CyExitCriticalSection(interruptState);
}


/*******************************************************************************
* Function Name: TransmitShiftReg_1_DisableInt
********************************************************************************
*
* Summary:
*  Disables the Shift Register interrupt.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void TransmitShiftReg_1_DisableInt(void) 
{
    uint8 interruptState;

    interruptState = CyEnterCriticalSection();
    TransmitShiftReg_1_SR_AUX_CONTROL &= ((uint8) ~TransmitShiftReg_1_INTERRUPTS_ENABLE);
    CyExitCriticalSection(interruptState);
}


/*******************************************************************************
* Function Name: TransmitShiftReg_1_GetFIFOStatus
********************************************************************************
*
* Summary:
*  Returns current status of input or output FIFO.
*
* Parameters:
*  fifoId.
*
* Return:
*  FIFO status.
*
*******************************************************************************/
uint8 TransmitShiftReg_1_GetFIFOStatus(uint8 fifoId) 
{
    uint8 result;

    result = TransmitShiftReg_1_RET_FIFO_NOT_DEFINED;

    #if(0u != TransmitShiftReg_1_USE_INPUT_FIFO)
        if(TransmitShiftReg_1_IN_FIFO == fifoId)
        {
            switch(TransmitShiftReg_1_GET_IN_FIFO_STS)
            {
                case TransmitShiftReg_1_IN_FIFO_FULL :
                    result = TransmitShiftReg_1_RET_FIFO_FULL;
                    break;

                case TransmitShiftReg_1_IN_FIFO_EMPTY :
                    result = TransmitShiftReg_1_RET_FIFO_EMPTY;
                    break;

                case TransmitShiftReg_1_IN_FIFO_PARTIAL:
                    result = TransmitShiftReg_1_RET_FIFO_PARTIAL;
                    break;
                    
                default:
                    /* Initial result value, while 
                       IN_FIFO_EMPTY case is false 
                     */
                    result = TransmitShiftReg_1_RET_FIFO_EMPTY;
                    break;
            }   
        }
    #endif /* (0u != TransmitShiftReg_1_USE_INPUT_FIFO) */

    if(TransmitShiftReg_1_OUT_FIFO == fifoId)
    {
        switch(TransmitShiftReg_1_GET_OUT_FIFO_STS)
        {
            case TransmitShiftReg_1_OUT_FIFO_FULL :
                result = TransmitShiftReg_1_RET_FIFO_FULL;
                break;

            case TransmitShiftReg_1_OUT_FIFO_EMPTY :
                result = TransmitShiftReg_1_RET_FIFO_EMPTY;
                break;

            case TransmitShiftReg_1_OUT_FIFO_PARTIAL :
                result = TransmitShiftReg_1_RET_FIFO_PARTIAL;
                break;

            default:
                /* Initial result value, while 
                   OUT_FIFO_FULL case is false 
                 */
                result = TransmitShiftReg_1_RET_FIFO_FULL;
                break;
        }
    }

    return(result);
}


/*******************************************************************************
* Function Name: TransmitShiftReg_1_SetIntMode
********************************************************************************
*
* Summary:
*  Sets the Interrupt Source for the Shift Register interrupt. Multiple
*  sources may be ORed together
*
* Parameters:
*  interruptSource: Byte containing the constant for the selected interrupt
*  source/s.
*
* Return:
*  None.
*
*******************************************************************************/
void TransmitShiftReg_1_SetIntMode(uint8 interruptSource) 
{
    TransmitShiftReg_1_SR_STATUS_MASK &= ((uint8) ~TransmitShiftReg_1_INTS_EN_MASK);          /* Clear existing int */
    TransmitShiftReg_1_SR_STATUS_MASK |= (interruptSource & TransmitShiftReg_1_INTS_EN_MASK); /* Set int */
}


/*******************************************************************************
* Function Name: TransmitShiftReg_1_GetIntStatus
********************************************************************************
*
* Summary:
*  Gets the Shift Register Interrupt status.
*
* Parameters:
*  None.
*
* Return:
*  Byte containing the constant for the selected interrupt source/s.
*
*******************************************************************************/
uint8 TransmitShiftReg_1_GetIntStatus(void) 
{
    return(TransmitShiftReg_1_SR_STATUS & TransmitShiftReg_1_INTS_EN_MASK);
}


/*******************************************************************************
* Function Name: TransmitShiftReg_1_WriteRegValue
********************************************************************************
*
* Summary:
*  Send state directly to shift register
*
* Parameters:
*  shiftData: containing shift register state.
*
* Return:
*  None.
*
*******************************************************************************/
void TransmitShiftReg_1_WriteRegValue(uint32 shiftData)
                                                                     
{
    CY_SET_REG32(TransmitShiftReg_1_SHIFT_REG_LSB_PTR, shiftData);
}


#if(0u != TransmitShiftReg_1_USE_INPUT_FIFO)
    /*******************************************************************************
    * Function Name: TransmitShiftReg_1_WriteData
    ********************************************************************************
    *
    * Summary:
    *  Send state to FIFO for later transfer to shift register based on the Load
    *  input
    *
    * Parameters:
    *  shiftData: containing shift register state.
    *
    * Return:
    *  Indicates: successful execution of function
    *  when FIFO is empty; and error when FIFO is full.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    cystatus TransmitShiftReg_1_WriteData(uint32 shiftData)
                                                                         
    {
        cystatus result;

        result = CYRET_INVALID_STATE;

        /* Writes data into the input FIFO if it is not FULL */
        if(TransmitShiftReg_1_RET_FIFO_FULL != (TransmitShiftReg_1_GetFIFOStatus(TransmitShiftReg_1_IN_FIFO)))
        {
            CY_SET_REG32(TransmitShiftReg_1_IN_FIFO_VAL_LSB_PTR, shiftData);
            result = CYRET_SUCCESS;
        }

        return(result);
    }
#endif /* (0u != TransmitShiftReg_1_USE_INPUT_FIFO) */


#if(0u != TransmitShiftReg_1_USE_OUTPUT_FIFO)
    /*******************************************************************************
    * Function Name: TransmitShiftReg_1_ReadData
    ********************************************************************************
    *
    * Summary:
    *  Returns state in FIFO due to Store input.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Shift Register state
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint32 TransmitShiftReg_1_ReadData(void) 
    {
        return(CY_GET_REG32(TransmitShiftReg_1_OUT_FIFO_VAL_LSB_PTR));
    }
#endif /* (0u != TransmitShiftReg_1_USE_OUTPUT_FIFO) */


/*******************************************************************************
* Function Name: TransmitShiftReg_1_ReadRegValue
********************************************************************************
*
* Summary:
*  Directly returns current state in shift register, not data in FIFO due
*  to Store input.
*
* Parameters:
*  None.
*
* Return:
*  Shift Register state. Clears output FIFO.
*
* Reentrant:
*  No.
*
*******************************************************************************/
uint32 TransmitShiftReg_1_ReadRegValue(void) 
{
    uint32 result;

    /* Clear FIFO before software capture */
    while(TransmitShiftReg_1_RET_FIFO_EMPTY != TransmitShiftReg_1_GetFIFOStatus(TransmitShiftReg_1_OUT_FIFO))
    {
        (void) CY_GET_REG32(TransmitShiftReg_1_OUT_FIFO_VAL_LSB_PTR);
    }

    /* Read of 8 bits from A1 causes capture to output FIFO */
    (void) CY_GET_REG8(TransmitShiftReg_1_SHIFT_REG_CAPTURE_PTR);

    /* Read output FIFO */
    result  = CY_GET_REG32(TransmitShiftReg_1_OUT_FIFO_VAL_LSB_PTR);
    
    #if(0u != (TransmitShiftReg_1_SR_SIZE % 8u))
        result &= ((uint32) TransmitShiftReg_1_SR_MASK);
    #endif /* (0u != (TransmitShiftReg_1_SR_SIZE % 8u)) */
    
    return(result);
}


/* [] END OF FILE */
