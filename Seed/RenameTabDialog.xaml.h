#pragma once

#include "RenameTabDialog.g.h"

namespace winrt::Seed::implementation
{
    struct RenameTabDialog : RenameTabDialogT<RenameTabDialog>
    {
    private:
        hstring m_tabUrl;
        hstring m_newTitle;

    public:
        RenameTabDialog()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        hstring TabUrl() const { return m_tabUrl; }
        void TabUrl(hstring const& value)
        {
            if (m_tabUrl != value)
            {
                m_tabUrl = value;
            }
        }

        hstring NewTitle() const { return m_newTitle; }
        void NewTitle(hstring const& value)
        {
            if (m_newTitle != value)
            {
                m_newTitle = value;
            }
        }
    };
}

namespace winrt::Seed::factory_implementation
{
    struct RenameTabDialog : RenameTabDialogT<RenameTabDialog, implementation::RenameTabDialog>
    {
    };
}
