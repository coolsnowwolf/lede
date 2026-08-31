/******************************************************************************
**
** FILE NAME    : ifxmips_pcie_pm.c
** PROJECT      : IFX UEIP
** MODULES      : PCIE Root Complex Driver
**
** DATE         : 21 Dec 2009
** AUTHOR       : Lei Chuanhua
** DESCRIPTION  : PCIE Root Complex Driver Power Managment
** COPYRIGHT    :       Copyright (c) 2009
**                      Lantiq Deutschland GmbH
**                      Am Campeon 3, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author         $Comment
** 21 Dec,2009   Lei Chuanhua    First UEIP release
*******************************************************************************/
/*!
  \defgroup IFX_PCIE_PM Power Management functions
  \ingroup IFX_PCIE
  \brief IFX PCIE Root Complex Driver power management functions
*/

/*!
 \file ifxmips_pcie_pm.c
 \ingroup IFX_PCIE
 \brief source file for PCIE Root Complex Driver Power Management
*/

#ifndef EXPORT_SYMTAB
#define EXPORT_SYMTAB
#endif
#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif /* AUTOCONF_INCLUDED */
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/system.h>

/* Project header */
#include <asm/ifx/ifx_types.h>
#include <asm/ifx/ifx_regs.h>
#include <asm/ifx/common_routines.h>
#include <asm/ifx/ifx_pmcu.h>
#include "ifxmips_pcie_pm.h"

/**
 * \fn static IFX_PMCU_RETURN_t ifx_pcie_pmcu_state_change(IFX_PMCU_STATE_t pmcuState)
 * \brief the callback function to request pmcu state in the power management hardware-dependent module
 *
 * \param pmcuState This parameter is a PMCU state.
 *
 * \return IFX_PMCU_RETURN_SUCCESS Set Power State successfully
 * \return IFX_PMCU_RETURN_ERROR   Failed to set power state.
 * \return IFX_PMCU_RETURN_DENIED  Not allowed to operate power state
 * \ingroup IFX_PCIE_PM
 */
static IFX_PMCU_RETURN_t
ifx_pcie_pmcu_state_change(IFX_PMCU_STATE_t pmcuState)
{
    switch(pmcuState)
    {
        case IFX_PMCU_STATE_D0:
            return IFX_PMCU_RETURN_SUCCESS;
        case IFX_PMCU_STATE_D1: // Not Applicable
            return IFX_PMCU_RETURN_DENIED;
        case IFX_PMCU_STATE_D2: // Not Applicable
            return IFX_PMCU_RETURN_DENIED;
        case IFX_PMCU_STATE_D3: // Module clock gating and Power gating
            return IFX_PMCU_RETURN_SUCCESS;
        default:
            return IFX_PMCU_RETURN_DENIED;
    }
}

/**
 * \fn static IFX_PMCU_RETURN_t ifx_pcie_pmcu_state_get(IFX_PMCU_STATE_t *pmcuState)
 * \brief the callback function to get pmcu state in the power management hardware-dependent module

 * \param pmcuState Pointer to return power state.
 *
 * \return IFX_PMCU_RETURN_SUCCESS Set Power State successfully
 * \return IFX_PMCU_RETURN_ERROR   Failed to set power state.
 * \return IFX_PMCU_RETURN_DENIED  Not allowed to operate power state
 * \ingroup IFX_PCIE_PM
 */
static IFX_PMCU_RETURN_t
ifx_pcie_pmcu_state_get(IFX_PMCU_STATE_t *pmcuState)
{
    return IFX_PMCU_RETURN_SUCCESS;
}

/**
 * \fn IFX_PMCU_RETURN_t ifx_pcie_pmcu_prechange(IFX_PMCU_MODULE_t pmcuModule, IFX_PMCU_STATE_t newState, IFX_PMCU_STATE_t oldState)
 * \brief Apply all callbacks registered to be executed before a state change for pmcuModule
 *
 * \param   pmcuModule      Module
 * \param   newState        New state
 * \param   oldState        Old state
 * \return  IFX_PMCU_RETURN_SUCCESS Set Power State successfully
 * \return  IFX_PMCU_RETURN_ERROR   Failed to set power state.
 * \ingroup IFX_PCIE_PM
 */
static IFX_PMCU_RETURN_t
ifx_pcie_pmcu_prechange(IFX_PMCU_MODULE_t pmcuModule, IFX_PMCU_STATE_t newState, IFX_PMCU_STATE_t oldState)
{
    return IFX_PMCU_RETURN_SUCCESS;
}

/**
 * \fn IFX_PMCU_RETURN_t ifx_pcie_pmcu_postchange(IFX_PMCU_MODULE_t pmcuModule, IFX_PMCU_STATE_t newState, IFX_PMCU_STATE_t oldState)
 * \brief Apply all callbacks registered to be executed before a state change for pmcuModule
 *
 * \param   pmcuModule      Module
 * \param   newState        New state
 * \param   oldState        Old state
 * \return IFX_PMCU_RETURN_SUCCESS Set Power State successfully
 * \return IFX_PMCU_RETURN_ERROR   Failed to set power state.
 * \ingroup IFX_PCIE_PM
 */
static IFX_PMCU_RETURN_t
ifx_pcie_pmcu_postchange(IFX_PMCU_MODULE_t pmcuModule, IFX_PMCU_STATE_t newState, IFX_PMCU_STATE_t oldState)
{
    return IFX_PMCU_RETURN_SUCCESS;
}

/**
 * \fn static void ifx_pcie_pmcu_init(void)
 * \brief Register with central PMCU module
 * \return none
 * \ingroup IFX_PCIE_PM
 */
void
ifx_pcie_pmcu_init(void)
{
    IFX_PMCU_REGISTER_t pmcuRegister;

    /* XXX, hook driver context */

    /* State function register */
    memset(&pmcuRegister, 0, sizeof(IFX_PMCU_REGISTER_t));
    pmcuRegister.pmcuModule = IFX_PMCU_MODULE_PCIE;
    pmcuRegister.pmcuModuleNr = 0;
    pmcuRegister.ifx_pmcu_state_change = ifx_pcie_pmcu_state_change;
    pmcuRegister.ifx_pmcu_state_get = ifx_pcie_pmcu_state_get;
    pmcuRegister.pre = ifx_pcie_pmcu_prechange;
    pmcuRegister.post= ifx_pcie_pmcu_postchange;
    ifx_pmcu_register(&pmcuRegister);
}

/**
 * \fn static void ifx_pcie_pmcu_exit(void)
 * \brief Unregister with central PMCU module
 *
 * \return none
 * \ingroup IFX_PCIE_PM
 */
void
ifx_pcie_pmcu_exit(void)
{
    IFX_PMCU_REGISTER_t pmcuUnRegister;

   /* XXX, hook driver context */

    pmcuUnRegister.pmcuModule = IFX_PMCU_MODULE_PCIE;
    pmcuUnRegister.pmcuModuleNr = 0;
    ifx_pmcu_unregister(&pmcuUnRegister);
}

