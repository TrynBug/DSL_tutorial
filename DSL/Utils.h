#pragma once

namespace dsl
{
    // exception을 발생시킬 때 wstring 메시지를 throw하기 위한 클래스
    class wexception : public std::exception 
    {
        
    public:
        wexception(const std::wstring& msg) 
            : m_msg(msg) 
        {}

        const char* what() const noexcept override 
        {
            return "wexception (use wwhat() for wide message)";
        }

        const std::wstring& wwhat() const noexcept 
        { 
            return m_msg;
        }

    private:
        std::wstring m_msg;
    };
}


