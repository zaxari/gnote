/*
 * gnote
 *
 * Copyright (C) 2013-2014 Aurimas Cernius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdexcept>

#include <boost/lexical_cast.hpp>

#include <glibmm/i18n.h>
#include <glibmm/keyfile.h>

#include "base/macros.hpp"
#include "addininfo.hpp"
#include "debug.hpp"
#include "sharp/string.hpp"


namespace gnote {

  namespace {

    const char * ADDIN_INFO = "AddinInfo";
    const char * ADDIN_ATTS = "AddinAttributes";

    AddinCategory resolve_addin_category(const std::string & cat)
    {
      if(cat == "Tools") {
        return ADDIN_CATEGORY_TOOLS;
      }
      if(cat == "Formatting") {
        return ADDIN_CATEGORY_FORMATTING;
      }
      if(cat == "DesktopIntegration") {
        return ADDIN_CATEGORY_DESKTOP_INTEGRATION;
      }
      if(cat == "Synchronization") {
        return ADDIN_CATEGORY_SYNCHRONIZATION;
      }

      return ADDIN_CATEGORY_UNKNOWN;
    }

  }



AddinInfo::AddinInfo(const std::string & info_file)
  : m_category(ADDIN_CATEGORY_UNKNOWN)
  , m_default_enabled(false)
{
  load_from_file(info_file);
}

void AddinInfo::load_from_file(const std::string & info_file)
{
  printf("%s: called %s\n", __func__, info_file.c_str());
  try {
    Glib::KeyFile addin_info;
    if(!addin_info.load_from_file(info_file)) {
      throw std::runtime_error(_("Failed to load plugin information!"));
    }
    m_id = addin_info.get_string(ADDIN_INFO, "Id");
    m_name = addin_info.get_locale_string(ADDIN_INFO, "Name");
    m_description = addin_info.get_locale_string(ADDIN_INFO, "Description");
    m_authors = addin_info.get_locale_string(ADDIN_INFO, "Authors");
    m_category = resolve_addin_category(addin_info.get_string(ADDIN_INFO, "Category"));
    m_version = addin_info.get_string(ADDIN_INFO, "Version");
    try {
      m_copyright = addin_info.get_locale_string(ADDIN_INFO, "Copyright");
    }
    catch(Glib::KeyFileError & e) {
      DBG_OUT("Can't read copyright, using none: %s", e.what().c_str());
    }
    try {
      m_default_enabled = addin_info.get_boolean(ADDIN_INFO, "DefaultEnabled");
    }
    catch(Glib::KeyFileError & e) {
      DBG_OUT("Can't read default enabled status, assuming default: %s", e.what().c_str());
    }
    m_addin_module = addin_info.get_string(ADDIN_INFO, "Module");
    m_libgnote_release = addin_info.get_string(ADDIN_INFO, "LibgnoteRelease");
    m_libgnote_version_info = addin_info.get_string(ADDIN_INFO, "LibgnoteVersionInfo");

    if(addin_info.has_group(ADDIN_ATTS)) {
      FOREACH(const Glib::ustring & key, addin_info.get_keys(ADDIN_ATTS)) {
        m_attributes[key] = addin_info.get_string(ADDIN_ATTS, key);
      }
    }
  }
  catch(Glib::Error & e) {
    throw std::runtime_error(e.what());
  }
}

Glib::ustring AddinInfo::get_attribute(const Glib::ustring & att)
{
  std::map<Glib::ustring, Glib::ustring>::iterator iter = m_attributes.find(att);
  if(iter != m_attributes.end()) {
    return iter->second;
  }
  return Glib::ustring();
}

bool AddinInfo::validate(const Glib::ustring & release, const Glib::ustring & version_info) const
{
  if(validate_compatibility(release, version_info)) {
    return true;
  }

  ERR_OUT(_("Incompatible plug-in %s: expected %s, got %s"),
          m_id.c_str(), (release + " " + version_info).c_str(),
          (m_libgnote_release + " " + m_libgnote_version_info).c_str());
  return false;
}

bool AddinInfo::validate_compatibility(const Glib::ustring & release, const Glib::ustring & version_info) const
{
  if(release != m_libgnote_release) {
    return false;
  }
  if(version_info == m_libgnote_version_info) {
    return true;
  }
  else {
    try {
      std::vector<std::string> parts;
      sharp::string_split(parts, m_libgnote_version_info, ":");
      if(parts.size() != 3) {
        return false;
      }

      int this_ver = boost::lexical_cast<int>(parts[0]);
      parts.clear();
      sharp::string_split(parts, version_info, ":");
      int ver = boost::lexical_cast<int>(parts[0]);
      int compat = boost::lexical_cast<int>(parts[2]);

      if(this_ver > ver) {
        // too new
        return false;
      }
      if(this_ver < ver - compat) {
        // too old
        return false;
      }

      return true;
    }
    catch(std::bad_cast &) {
      return false;
    }
  }
}


}
