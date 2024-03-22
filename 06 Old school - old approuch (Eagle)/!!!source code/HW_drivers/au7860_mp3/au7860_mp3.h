#ifndef __au7860_mp3__ 
   #define  __au7860_mp3__
   
   #define  au_header                           0x55AA
   
   #define  au_address_len                      0x2
   #define  au_address_command                  0x3
   #define  au_address_data                     0x5
   //check sum addres is au_address_data + data_len
   
   #define  au_command_Mode_Next                0x0101
   #define  au_command_Mode_Select              0x0102
   #define  au_command_Mode_Get                 0x0103
   #define  au_command_Mode_Get_device_link     0x0104
   
   #define  au_command_Mode_Next_len            0x0
   #define  au_command_Mode_Select_len          0x1
   #define  au_command_Mode_Get_len             0x0
   #define  au_command_Mode_Get_device_link_len 0x0  
   
   #define  au_command_FS_GetFolderSum          0x0201
   #define  au_command_FS_GetFileSum            0x0202
   #define  au_command_FS_GetFolderInfo         0x0203
   #define  au_command_FS_GetFileInfo           0x0204
   
   #define  au_command_FS_GetFolderSum_len      0x0      
   #define  au_command_FS_GetFileSum_len        0x0
   #define  au_command_FS_GetFolderInfo_len     0x4
   #define  au_command_FS_GetFileInfo_len       0x4 
   
   #define  au_command_Play_Play                0x0301
   #define  au_command_Play_Pause               0x0302
   #define  au_command_Play_Stop                0x0303
   #define  au_command_Play_Next                0x0304
   #define  au_command_Play_Pre                 0x0305
   #define  au_command_Play_PlayPause           0x0306
   #define  au_command_Play_FF                  0x0307
   #define  au_command_Play_FB                  0x0308
   #define  au_command_Play_StopFFFB            0x0309
   #define  au_command_Play_SeekPlayTime        0x030A
   #define  au_command_Play_NextFolder          0x030B
   #define  au_command_Play_PreFolder           0x030C
   #define  au_command_Play_SelectSong          0x030D
   #define  au_command_Play_GetPlayStatus       0x0320
   #define  au_command_Play_GetSongInfo         0x0321
   #define  au_command_Play_SetRepeatMode       0x0322
   #define  au_command_Play_GetRepeatMode       0x0323
   #define  au_command_Play_EnableFolder        0x0324
   #define  au_command_Play_DisableFolder       0x0325
   #define  au_command_Play_GetTitle            0x0330
   #define  au_command_Play_GetArtist           0x0331
   #define  au_command_Play_GetAlbum            0x0332
   #define  au_command_Play_GetComment          0x0333
   #define  au_command_Play_GetYear             0x0334
   
   #define  au_command_Play_Play_len            0x0
   #define  au_command_Play_Pause_len           0x0
   #define  au_command_Play_Stop_len            0x0
   #define  au_command_Play_Next_len            0x0
   #define  au_command_Play_Pre_len             0x0
   #define  au_command_Play_PlayPause_len       0x0
   #define  au_command_Play_FF_len              0x0
   #define  au_command_Play_FB_len              0x0
   #define  au_command_Play_StopFFFB_len        0x0
   #define  au_command_Play_SeekPlayTime_len    0x4
   #define  au_command_Play_NextFolder_len      0x0
   #define  au_command_Play_PreFolder_len       0x0
   #define  au_command_Play_SelectSong_len      0x4
   #define  au_command_Play_GetPlayStatus_len   0x0
   #define  au_command_Play_GetSongInfo_len     0x0
   #define  au_command_Play_SetRepeatMode_len   0x1
   #define  au_command_Play_GetRepeatMode_len   0x0
   #define  au_command_Play_EnableFolder_len    0x0
   #define  au_command_Play_DisableFolder_len   0x0
   #define  au_command_Play_GetTitle_len        0x0
   #define  au_command_Play_GetArtist_len       0x0
   #define  au_command_Play_GetAlbum_len        0x0
   #define  au_command_Play_GetComment_len      0x0
   #define  au_command_Play_GetYear_len         0x0
   
   #define  au_command_Audio_VolPlus            0x0401
   #define  au_command_Audio_VolMinus           0x0402
   #define  au_command_Audio_VolSet             0x0403
   #define  au_command_Audio_VolGet             0x0404
   #define  au_command_Audio_EqSet              0x0405
   #define  au_command_Audio_EqGet              0x0406
   #define  au_command_Audio_Mute               0x0407
   #define  au_command_Audio_UnMute             0x0408
   #define  au_command_Audio_SetLineInGain      0x0409
   #define  au_command_Audio_EnableMic          0x040A
   #define  au_command_Audio_DisableMic         0x040B
   
   #define  au_command_Audio_VolPlus_len        0x0
   #define  au_command_Audio_VolMinus_len       0x0
   #define  au_command_Audio_VolSet_len         0x1
   #define  au_command_Audio_VolGet_len         0x0
   #define  au_command_Audio_EqSet_len          0x1
   #define  au_command_Audio_EqGet_len          0x0
   #define  au_command_Audio_Mute_len           0x0
   #define  au_command_Audio_UnMute_len         0x0
   #define  au_command_Audio_SetLineInGain_len  0x1
   #define  au_command_Audio_EnableMic_len      0x1
   #define  au_command_Audio_DisableMic_len     0x0

   #define  au_cmd_val_ModeID_Idle              0x0
   #define  au_cmd_val_ModeID_USB               0x1
   #define  au_cmd_val_ModeID_SD                0x2
   #define  au_cmd_val_ModeID_AuxIn             0x3
   #define  au_cmd_val_ModeID_USBAudio          0x4

   #define  au_cmd_rep_Linked_USB               0b00000001
   #define  au_cmd_rep_Linked_SD                0b00000010
   #define  au_cmd_rep_Linked_AuxIn             0b00000100
   #define  au_cmd_rep_Linked_PC                0b00001000





#endif
