define(`confCC', `TARGET_CC')
define(`confCCOPTS', `TARGET_CFLAGS')
APPENDDEF(`confENVDEF',`-DSTARTTLS')
APPENDDEF(`confLIBS', `-lssl -lcrypto')

