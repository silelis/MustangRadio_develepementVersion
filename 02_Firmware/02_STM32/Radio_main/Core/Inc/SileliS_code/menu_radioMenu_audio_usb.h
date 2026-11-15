/*
 * menu_radioMenu_audio_usb.h
 *
 *  Created on: 15 lis 2026
 *      Author: dbank
 */

#ifndef INC_SILELIS_CODE_MENU_RADIOMENU_AUDIO_USB_H_
#define INC_SILELIS_CODE_MENU_RADIOMENU_AUDIO_USB_H_


void radioMenuAudioUsb_Init(void *context);			//context: radioMenu* (z myList::bindCurrent / execCurrent*)
void radioMenuAudioUsb_ReturnInit(void *context);
void radioMenuAudioUsb_DestroyInit(void *context);


#endif /* INC_SILELIS_CODE_MENU_RADIOMENU_AUDIO_USB_H_ */
