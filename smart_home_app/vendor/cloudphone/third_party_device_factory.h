/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-10
** Description 
**  
***************************************************************************/

#ifndef _THIRD_PARTY_DEVICE_FACTORY_H_
#define _THIRD_PARTY_DEVICE_FACTORY_H_

#include "common_types.h"
#include "third_party_device.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace cmcc_webrtc {

class ThirdpartyAudioDevice;

class ThirdpartyAudioDeviceFactory {
 public:
  static ThirdpartyAudioDevice* CreateThirdpartyAudioDevice(const int32_t id);

  // The input file must be a readable 48k stereo raw file. The output
  // file must be writable. The strings will be copied.
  static void SetFilenamesToUse(const char* inputAudioFilename,
                                const char* outputAudioFilename);

 private:
  static const uint32_t MAX_FILENAME_LEN = 256;
  static char _inputAudioFilename[MAX_FILENAME_LEN];
  static char _outputAudioFilename[MAX_FILENAME_LEN];
};

}  // namespace cmcc_webrtc

#ifdef __cplusplus
}
#endif
#endif


