/******************************************************************************
**
** FILE NAME    : ifxmips_deu.c
** PROJECT      : IFX UEIP
** MODULES      : DEU Module for Danube
**
** DATE         : September 8, 2009
** AUTHOR       : Mohammad Firdaus
** DESCRIPTION  : Data Encryption Unit Driver
** COPYRIGHT    :       Copyright (c) 2009
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author             $Comment
** 08,Sept 2009 Mohammad Firdaus    Initial UEIP release
*******************************************************************************/

/*!
  \defgroup IFX_DEU IFX_DEU_DRIVERS
  \ingroup API
  \brief ifx deu driver module
*/

/*!
  \file	ifxmips_deu.c
  \ingroup IFX_DEU
  \brief main deu driver file
*/

/*!
 \defgroup IFX_DEU_FUNCTIONS IFX_DEU_FUNCTIONS
 \ingroup IFX_DEU
 \brief IFX DEU functions
*/

/* Project header */
#include <linux/version.h>
#if defined(CONFIG_MODVERSIONS)
#define MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/crypto.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>
#include <linux/fs.h>       /* Stuff about file systems that we need */
#include <asm/byteorder.h>
#include "ifxmips_deu.h"

#include <lantiq_soc.h>

#if defined(CONFIG_DANUBE)
#include "ifxmips_deu_danube.h"
#elif defined(CONFIG_AR9)
#include "ifxmips_deu_ar9.h"
#elif defined(CONFIG_VR9) || defined(CONFIG_AR10)
#include "ifxmips_deu_vr9.h"
#else 
#error "Platform unknown!"
#endif /* CONFIG_xxxx */

int disable_deudma = 1;

void chip_version(void);

/*! \fn static int __init deu_init (void)
 *  \ingroup IFX_DEU_FUNCTIONS
 *  \brief link all modules that have been selected in kernel config for ifx hw crypto support   
 *  \return ret 
*/  
                               
static int ltq_deu_probe(struct platform_device *pdev)
{
    int ret = -ENOSYS;


    START_DEU_POWER;
    
#define IFX_DEU_DRV_VERSION         "2.0.0"
         printk(KERN_INFO "Infineon Technologies DEU driver version %s \n", IFX_DEU_DRV_VERSION);

    FIND_DEU_CHIP_VERSION;

#if defined(CONFIG_CRYPTO_DEV_DES)
    if ((ret = ifxdeu_init_des ())) {
        printk (KERN_ERR "IFX DES initialization failed!\n");
    }
#endif
#if defined(CONFIG_CRYPTO_DEV_AES)
    if ((ret = ifxdeu_init_aes ())) {
        printk (KERN_ERR "IFX AES initialization failed!\n");
    }

#endif
#if defined(CONFIG_CRYPTO_DEV_ARC4)
    if ((ret = ifxdeu_init_arc4 ())) {
        printk (KERN_ERR "IFX ARC4 initialization failed!\n");
    }

#endif
#if defined(CONFIG_CRYPTO_DEV_SHA1)
    if ((ret = ifxdeu_init_sha1 ())) {
        printk (KERN_ERR "IFX SHA1 initialization failed!\n");
    }
#endif
#if defined(CONFIG_CRYPTO_DEV_MD5)
    if ((ret = ifxdeu_init_md5 ())) {
        printk (KERN_ERR "IFX MD5 initialization failed!\n");
    }

#endif
#if defined(CONFIG_CRYPTO_DEV_SHA1_HMAC)
    if ((ret = ifxdeu_init_sha1_hmac ())) {
        printk (KERN_ERR "IFX SHA1_HMAC initialization failed!\n");
    }
#endif
#if defined(CONFIG_CRYPTO_DEV_MD5_HMAC)
    if ((ret = ifxdeu_init_md5_hmac ())) {
        printk (KERN_ERR "IFX MD5_HMAC initialization failed!\n");
    }
#endif



    return ret;

}

/*! \fn static void __exit deu_fini (void)
 *  \ingroup IFX_DEU_FUNCTIONS
 *  \brief remove the loaded crypto algorithms   
*/                                 
static int ltq_deu_remove(struct platform_device *pdev)
{
//#ifdef CONFIG_CRYPTO_DEV_PWR_SAVE_MODE
    #if defined(CONFIG_CRYPTO_DEV_DES)
    ifxdeu_fini_des ();
    #endif
    #if defined(CONFIG_CRYPTO_DEV_AES)
    ifxdeu_fini_aes ();
    #endif
    #if defined(CONFIG_CRYPTO_DEV_ARC4)
    ifxdeu_fini_arc4 ();
    #endif
    #if defined(CONFIG_CRYPTO_DEV_SHA1)
    ifxdeu_fini_sha1 ();
    #endif
    #if defined(CONFIG_CRYPTO_DEV_MD5)
    ifxdeu_fini_md5 ();
    #endif
    #if defined(CONFIG_CRYPTO_DEV_SHA1_HMAC)
    ifxdeu_fini_sha1_hmac ();
    #endif
    #if defined(CONFIG_CRYPTO_DEV_MD5_HMAC)
    ifxdeu_fini_md5_hmac ();
    #endif
    printk("DEU has exited successfully\n");

	return 0;
}


int disable_multiblock = 0;

module_param(disable_multiblock,int,0);


static const struct of_device_id ltq_deu_match[] = {
#ifdef CONFIG_DANUBE
	{ .compatible = "lantiq,deu-danube"},
#elif defined CONFIG_AR9
	{ .compatible = "lantiq,deu-arx100"},
#elif defined CONFIG_VR9
	{ .compatible = "lantiq,deu-xrx200"},
#endif
	{},
};
MODULE_DEVICE_TABLE(of, ltq_deu_match);


static struct platform_driver ltq_deu_driver = {
	.probe = ltq_deu_probe,
	.remove = ltq_deu_remove,
	.driver = {
		.name = "deu",
		.owner = THIS_MODULE,
		.of_match_table = ltq_deu_match,
	},
};

module_platform_driver(ltq_deu_driver);

MODULE_DESCRIPTION ("Infineon DEU crypto engine support.");
MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Mohammad Firdaus");
