#ifndef XPCONFIG_H
#define XPCONFIG_H

#include <string>
#include <map>

using namespace std;

class XPConfig {
    public:
        static XPConfig* Instance();
        bool OpenConfigFile(std::string configFile);
        void LoadDefaults();
        void SetBool(string param, bool option);
        bool GetBool(string param);

    private:
        XPConfig(){};  // Private so that it can  not be called
        XPConfig(XPConfig const&){};             // copy constructor is private
        void operator=(XPConfig const&){};  // assignment operator is private
        static XPConfig* m_pInstance;

        map<string, bool> boolMap;
};

#endif