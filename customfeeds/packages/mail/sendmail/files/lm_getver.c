#include <stdio.h>
#include "libmilter/mfapi.h"
int main() {
	printf("%d.%d.%d",
		SM_LM_VRS_MAJOR(SMFI_VERSION),
		SM_LM_VRS_MINOR(SMFI_VERSION),
		SM_LM_VRS_PLVL(SMFI_VERSION)
	);
}
