#pragma once
#include <string>
#include <map>

class LanguageManager
{
public:
    static LanguageManager& GetInstance();
    void SetLanguage(const std::string& langCode);
    std::string GetCurrentLanguage() const;
    const char* Get(const std::string& key) const;

private:
    LanguageManager();
    void LoadStrings();
    void LoadChinese();
    void LoadEnglish();

    std::string m_currentLangCode;
    std::map<std::string, std::string> m_strings;
};

#define LANG(key) LanguageManager::GetInstance().Get(key)
