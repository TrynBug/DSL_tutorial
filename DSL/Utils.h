#pragma once

namespace dsl
{
    // exception�� �߻���ų �� wstring �޽����� throw�ϱ� ���� Ŭ����
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


