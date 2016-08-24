/*
 *      Copyright (C) 2007-2014 Team Kodi
 *      Copyright (C) 2016 Team Kodi
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

/*
xmms-projectM v0.99 - xmms-projectm.sourceforge.net
--------------------------------------------------

Lead Developers:  Carmelo Piccione (cep@andrew.cmu.edu) &
                  Peter Sperl (peter@sperl.com)

We have also been advised by some professors at CMU, namely Roger B. Dannenberg.
http://www-2.cs.cmu.edu/~rbd/

The inspiration for this program was Milkdrop by Ryan Geiss. Obviously.

This code is distributed under the GPL.


THANKS FOR THE CODE!!!
-------------------------------------------------
The base for this program was andy@nobugs.org's XMMS plugin tutorial
http://www.xmms.org/docs/vis-plugin.html

We used some FFT code by Takuya OOURA instead of XMMS' built-in fft code
fftsg.c - http://momonga.t.u-tokyo.ac.jp/~ooura/fft.html

For font rendering we used GLF by Roman Podobedov
glf.c - http://astronomy.swin.edu.au/~pbourke/opengl/glf/

and some beat detection code was inspired by Frederic Patin @
www.gamedev.net/reference/programming/features/beatdetection/
--

"ported" to Kodi by d4rk
d4rk@xbmc.org

*/

#include "Main.h"

#include <kodi/api2/visualization/Addon.hpp>
#if !defined(__APPLE__)
#include <GL/glew.h>
#endif

using namespace Visualization;

class CVisualizationProjectM : public Visualization::CAddonInterface
{
public:
  CVisualizationProjectM(CAddonVisualizationProjectM *addon, void* kodiInstance);
  virtual ~CVisualizationProjectM();

  virtual void AudioData(const float* pAudioData, int iAudioDataLength, float* pFreqData, int iFreqDataLength);
  virtual void Render();
  virtual void GetInfo(Visualization::sInfo& info);
  virtual bool OnAction(long flags, const void* param);
  virtual bool GetPresets(std::vector<std::string>& presets);
  virtual unsigned int GetPreset();
  virtual bool IsLocked();

  static projectM *m_projectM;

private:
  bool InitProjectM();
  
  int m_maxSamples;
  int m_texsize;
  int m_gx, m_gy;
  int m_fps;

  CAddonVisualizationProjectM* m_addon;
  unsigned int m_lastLoggedPresetIdx;
  std::vector<std::string> m_presets;
};

projectM *CVisualizationProjectM::m_projectM = nullptr;

CVisualizationProjectM::CVisualizationProjectM(CAddonVisualizationProjectM *addon, void* kodiInstance)
  : Visualization::CAddonInterface(kodiInstance),
    m_maxSamples(512),
    m_texsize(512),
    m_gx(40),
    m_gy(30),
    m_fps(100),
    m_addon(addon)
{
  m_addon->m_configPM.meshX = m_gx;
  m_addon->m_configPM.meshY = m_gy;
  m_addon->m_configPM.fps = m_fps;
  m_addon->m_configPM.textureSize = m_texsize;
  m_addon->m_configPM.windowWidth = GetProperties()->width;
  m_addon->m_configPM.windowHeight = GetProperties()->height;
  m_addon->m_configPM.aspectCorrection = true;
  m_addon->m_configPM.easterEgg = 0.0;
  std::string path = GetAddonPath();
  path += "/resources";
  m_addon->m_configPM.titleFontURL = path;
  m_addon->m_configPM.titleFontURL += "/Vera.ttf";
  m_addon->m_configPM.menuFontURL = path;
  m_addon->m_configPM.menuFontURL += "/VeraMono.ttf";
  m_lastLoggedPresetIdx = m_addon->m_lastPresetIdx;

  InitProjectM();
}

CVisualizationProjectM::~CVisualizationProjectM()
{
  if (m_projectM)
  {
    delete m_projectM;
    m_projectM = nullptr;
  }
}

//-- Audiodata ----------------------------------------------------------------
// Called by Kodi to pass new audio data to the vis
//-----------------------------------------------------------------------------
void CVisualizationProjectM::AudioData(const float* pAudioData, int iAudioDataLength, float* pFreqData, int iFreqDataLength)
{
  if (m_projectM)
    m_projectM->pcm()->addPCMfloat(pAudioData, iAudioDataLength); 
}

//-- Render -------------------------------------------------------------------
// Called once per frame. Do all rendering here.
//-----------------------------------------------------------------------------
void CVisualizationProjectM::Render() 
{
  if (m_projectM)
  {
    m_projectM->renderFrame();
    unsigned preset;
    m_projectM->selectedPresetIndex(preset);
//    if (m_lastLoggedPresetIdx != preset)
//      CLog::Log(LOGDEBUG,"PROJECTM - Changed preset to: %s",m_presets[preset]);
    m_lastLoggedPresetIdx = preset;
  }
}

//-- GetInfo ------------------------------------------------------------------
// Tell Kodi our requirements
//-----------------------------------------------------------------------------
void CVisualizationProjectM::GetInfo(Visualization::sInfo& info) 
{
  info.bWantsFreq = false;
  info.iSyncDelay = 0;
}

//-- OnAction -----------------------------------------------------------------
// Handle Kodi actions such as next preset, lock preset, album art changed etc
//-----------------------------------------------------------------------------
bool CVisualizationProjectM::OnAction(long flags, const void* param)
{
  if (!m_projectM)
    return false;

  switch (flags)
  {
    case VIS_ACTION_LOAD_PRESET:
      if (param)
      {
        int pindex = *((int *)param);
        m_projectM->selectPreset(pindex);
        return true;
      }
      break;
    case VIS_ACTION_NEXT_PRESET:
  //    switchPreset(ALPHA_NEXT, SOFT_CUT);
      if (!m_projectM->isShuffleEnabled())
        m_projectM->key_handler(PROJECTM_KEYDOWN, PROJECTM_K_n, PROJECTM_KMOD_CAPS); //ignore PROJECTM_KMOD_CAPS
      else
        m_projectM->key_handler(PROJECTM_KEYDOWN, PROJECTM_K_r, PROJECTM_KMOD_CAPS); //ignore PROJECTM_KMOD_CAPS
      return true;
    case VIS_ACTION_PREV_PRESET:
  //    switchPreset(ALPHA_PREVIOUS, SOFT_CUT);
      if (!m_projectM->isShuffleEnabled())
        m_projectM->key_handler(PROJECTM_KEYDOWN, PROJECTM_K_p, PROJECTM_KMOD_CAPS); //ignore PROJECTM_KMOD_CAPS
      else
        m_projectM->key_handler(PROJECTM_KEYDOWN, PROJECTM_K_r, PROJECTM_KMOD_CAPS); //ignore PROJECTM_KMOD_CAPS
      return true;
    case VIS_ACTION_RANDOM_PRESET:
      m_projectM->setShuffleEnabled(m_addon->m_configPM.shuffleEnabled);
      return true;
    case VIS_ACTION_LOCK_PRESET:
      m_projectM->setPresetLock(!m_projectM->isPresetLocked());
      unsigned preset;
      m_projectM->selectedPresetIndex(preset);
      m_projectM->selectPreset(preset);
      return true;
    default:
      break;
  }
  return false;
}

bool CVisualizationProjectM::InitProjectM()
{
  if (m_projectM)
  {
    delete m_projectM; //We are re-initalizing the engine
    m_projectM = nullptr;
  }
  try
  {
    m_projectM = new projectM(m_addon->m_configPM);
    if (m_addon->m_configPM.presetURL == m_addon->m_lastPresetDir)  //If it is not the first run AND if this is the same preset pack as last time
    {
      m_projectM->setPresetLock(m_addon->m_lastLockStatus);
      m_projectM->selectPreset(m_addon->m_lastLockStatus);
    }
    else
    {
      // If it is the first run or a newly chosen preset pack we choose a random preset as first
      if (m_projectM->getPlaylistSize())
        m_projectM->selectPreset((rand() % (m_projectM->getPlaylistSize())));
    }
    return true;
  }
  catch (...)
  {
    printf("exception in projectM ctor");
    return false;
  }
}


//-- GetPresets ---------------------------------------------------------------
// Return a list of presets to Kodi for display
//-----------------------------------------------------------------------------
bool CVisualizationProjectM::GetPresets(std::vector<std::string>& presets)
{
  unsigned int numPresets = m_projectM ? m_projectM->getPlaylistSize() : 0;
  if (numPresets > 0)
  {
    for (unsigned i = 0; i < numPresets; i++)
      presets.push_back(m_projectM->getPresetName(i));
  }
  return (numPresets > 0);
}

//-- GetPreset ----------------------------------------------------------------
// Return the index of the current playing preset
//-----------------------------------------------------------------------------
unsigned int CVisualizationProjectM::GetPreset()
{ 
  unsigned preset;
  if(m_projectM && m_projectM->selectedPresetIndex(preset))
    return preset;
  return 0;
}

//-- IsLocked -----------------------------------------------------------------
// Returns true if this add-on use settings
//-----------------------------------------------------------------------------
bool CVisualizationProjectM::IsLocked()
{
  if(m_projectM)
    return m_projectM->isPresetLocked();
  else
    return false;
}

eAddonStatus CAddonVisualizationProjectM::CreateInstance(eInstanceType instanceType, KODI_HANDLE* addonInstance, KODI_HANDLE kodiInstance)
{
  KodiAPI::Log(ADDON_LOG_DEBUG, "%s - Creating the Project M visualization add-on", __FUNCTION__);

  if (instanceType != CAddon::instanceVisualization)
  {
    KodiAPI::Log(ADDON_LOG_FATAL, "%s - Creation called with unsupported instance type", __FUNCTION__);
    return addonStatus_PERMANENT_FAILURE;
  }

  *addonInstance = new CVisualizationProjectM(this, kodiInstance);
  return addonStatus_OK;
}

void CAddonVisualizationProjectM::DestroyInstance(KODI_HANDLE addonInstance)
{
  delete static_cast<CVisualizationProjectM*>(addonInstance);
}

//-- UpdateSetting ------------------------------------------------------------
// Handle setting change request from Kodi
//-----------------------------------------------------------------------------
eAddonStatus CAddonVisualizationProjectM::SetSetting(std::string& settingName, const void *settingValue)
{
  if (settingName.empty() || settingValue == nullptr)
    return addonStatus_UNKNOWN;

  if (settingName == "###GetSavedSettings") // We have some settings to be saved in the settings.xml file
  {
    if (!CVisualizationProjectM::m_projectM)
    {
      return addonStatus_UNKNOWN;
    }
    if (strcmp((char*)settingValue, "0") == 0)
    {
      settingName = "lastpresetfolder";
      strcpy((char*)settingValue, CVisualizationProjectM::m_projectM->settings().presetURL.c_str());
    }
    if (strcmp((char*)settingValue, "1") == 0)
    {
      settingName = "lastlockedstatus";
      strcpy((char*)settingValue, (CVisualizationProjectM::m_projectM->isPresetLocked() ? "true" : "false"));
    }
    if (strcmp((char*)settingValue, "2") == 0)
    {
      settingName = "lastpresetidx";
      unsigned int lastindex;
      CVisualizationProjectM::m_projectM->selectedPresetIndex(lastindex);
      sprintf ((char*)settingValue, "%i", (int)lastindex);
    }
    if (strcmp((char*)settingValue, "3") == 0)
    {
      settingName = "###End";
    }
    return addonStatus_OK;
  }
  // It is now time to set the settings got from xmbc
  if (settingName == "quality")
    ChooseQuality (*(int*)settingValue);
  else if (settingName == "shuffle")
    m_configPM.shuffleEnabled == *(bool*)settingValue;
  
  else if (settingName == "lastpresetidx")
    m_lastPresetIdx = *(int*)settingValue;
  else if (settingName == "lastlockedstatus")
    m_lastLockStatus = *(bool*)settingValue;
  else if (settingName == "lastpresetfolder")
    m_lastPresetDir = (char*)settingValue;
  else if (settingName == "smooth_duration")
    m_configPM.smoothPresetDuration = (*(int*)settingValue * 5 + 5);
  else if (settingName == "preset_duration")
    m_configPM.presetDuration = (*(int*)settingValue * 5 + 5);
  else if (settingName == "preset pack")
    ChoosePresetPack(*(int*)settingValue);
  else if (settingName == "user preset folder")
    ChooseUserPresetFolder((char*)settingValue);
  else if (settingName == "beat_sens")
    m_configPM.beatSensitivity = *(int*)settingValue * 2;

  return addonStatus_OK;
}

void CAddonVisualizationProjectM::ChooseQuality(int pvalue)
{
  switch (pvalue)
  {
    case 0:
      m_configPM.textureSize = 256;
      break;
    case 1:
      m_configPM.textureSize = 512;
      break;
    case 2:
      m_configPM.textureSize = 1024;
      break;
    case 3:
      m_configPM.textureSize = 2048;
      break;
  }
}

void CAddonVisualizationProjectM::ChoosePresetPack(int pvalue)
{
  m_UserPackFolder = false;
   if (pvalue == 0)
   {
     m_configPM.presetURL = GetAddonPath();
     m_configPM.presetURL += "/resources/presets";
   }
   else if (pvalue == 1) //User preset folder has been chosen
     m_UserPackFolder = true;
}

void CAddonVisualizationProjectM::ChooseUserPresetFolder(std::string pvalue)
{
  if (m_UserPackFolder)
  {
    pvalue.erase(pvalue.length()-1,1);  //Remove "/" from the end
    m_configPM.presetURL = pvalue;
  }
}
    
ADDONCREATOR(CAddonVisualizationProjectM); // Don't touch this!
