#include "XPConfig.hpp"

// Global static pointer used to ensure a single instance of the class.
XPConfig* XPConfig::m_pInstance = NULL;  

/** This function is called to create an instance of the class. 
    Calling the constructor publicly is not allowed. The constructor 
    is private and is only called by this Instance function.
*/
  
XPConfig* XPConfig::Instance()
{
   if (!m_pInstance)   // Only allow one instance of class to be generated.
      m_pInstance = new XPConfig;

   return m_pInstance;
}

bool XPConfig::OpenConfigFile(std::string configFile)
{
    LoadDefaults();
    //Use file
    return false;
}

void XPConfig::LoadDefaults()
{
    //Default
    boolMap["blockFreeriders"] = false;
    boolMap["playerEnabled"] = true;
    boolMap["clientLogsEnabled"] = false;
    boolMap["leakyBucketDataFilter"] = false;
    boolMap["serverCandidate"] = false;     //ECM
}

void XPConfig::SetBool(string param, bool option)
{
    boolMap[param] = option;
}

bool XPConfig::GetBool(string param)
{
    return boolMap[param];
}
