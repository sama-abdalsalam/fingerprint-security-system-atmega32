#ifndef HAL_FINGERPRINT_FINGERPRINT_INTERFACE_H_
#define HAL_FINGERPRINT_FINGERPRINT_INTERFACE_H_

#include "../../STD_TYPES.h"

u8 FINGERPRINT_CheckConnection(void);
u8 FINGERPRINT_Enroll(u8 page_id);
u8 FINGERPRINT_Search(u8 *found_id);
u8 FINGERPRINT_Delete(u8 page_id);

#endif
