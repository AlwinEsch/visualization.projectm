#pragma once
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

#include <kodi/api2/Addon.hpp>
#include "libprojectM/projectM.hpp"

class CAddonVisualizationProjectM : public CAddon
{
public:
  CAddonVisualizationProjectM() 
    : m_UserPackFolder(false) { }

  virtual eAddonStatus Create() override { return addonStatus_NEED_SAVEDSETTINGS; }
  virtual eAddonStatus CreateInstance(eInstanceType instanceType, KODI_HANDLE* addonInstance, KODI_HANDLE kodiInstance) override;
  virtual void DestroyInstance(KODI_HANDLE addonInstance) override;
  virtual bool HasSettings() override { return true; }
  virtual eAddonStatus SetSetting(std::string& settingName, const void *settingValue) override;

  projectM::Settings m_configPM;
  int m_lastPresetIdx;
  bool m_lastLockStatus;
  bool m_UserPackFolder;
  std::string m_lastPresetDir;

private:
  void ChooseQuality(int pvalue);
  void ChoosePresetPack(int pvalue);
  void ChooseUserPresetFolder(std::string pvalue);
};
