
#include "pch.h"
#include "ApplicationViewModel.h"
#include "Common/TraceLogger.h"
#include "Common/AppResourceProvider.h"
#include "StandardCalculatorViewModel.h"
#include "DateCalculatorViewModel.h"
#include "DataLoaders/CurrencyHttpClient.h"
#include "DataLoaders/CurrencyDataLoader.h"
#include "DataLoaders/UnitConverterDataLoader.h"

using namespace CalculatorApp;
using namespace CalculatorApp::Common;
using namespace CalculatorApp::DataLoaders;
using namespace CalculatorApp::ViewModel;
using namespace CalculationManager;
using namespace Platform;
using namespace Platform::Collections;
using namespace std;
using namespace Windows::System;
using namespace Windows::Storage;
using namespace Utils;
using namespace Windows::Foundation::Collections;
using namespace Windows::Globalization;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Automation;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::Foundation;
using namespace Concurrency;

namespace
{
    StringReference CategoriesPropertyName(L"Categories");
    StringReference ClearMemoryVisibilityPropertyName(L"ClearMemoryVisibility");
}

ViewModelDisplay::ViewModelDisplay()
    : m_CalculatorViewModel(nullptr)
    , m_DateCalcViewModel(nullptr)
    , m_ConverterViewModel(nullptr)
    , m_PreviousMode(ViewMode::None)
    , m_mode(ViewMode::None)
    , m_categories(nullptr)
{
    SetMenuCategories();
}

void ApplicationViewModel::Mode::set(ViewMode value)
{

void ApplicationViewModel::ToggleAlwaysOnTop(float width, float height)
{
    HandleToggleAlwaysOnTop(width, height);
}

#pragma optimize("", off)
task<void> ApplicationViewModel::HandleToggleAlwaysOnTop(float width, float height)
{
    if (ApplicationView::GetForCurrentView()->ViewMode == ApplicationViewMode::CompactOverlay)
    {
        ApplicationDataContainer ^ localSettings = ApplicationData::Current->LocalSettings;
        localSettings->Values->Insert(WidthLocalSettings, width);
        localSettings->Values->Insert(HeightLocalSettings, height);

        bool success = co_await ApplicationView::GetForCurrentView()->TryEnterViewModeAsync(ApplicationViewMode::Default);
        CalculatorViewModel->AreHistoryShortcutsEnabled = success;
        CalculatorViewModel->HistoryVM->AreHistoryShortcutsEnabled = success;
        CalculatorViewModel->IsAlwaysOnTop = !success;
        IsAlwaysOnTop = !success;
    }
    else
    {
        ApplicationDataContainer ^ localSettings = ApplicationData::Current->LocalSettings;
        ViewModePreferences ^ compactOptions = ViewModePreferences::CreateDefault(ApplicationViewMode::CompactOverlay);
        if (!localSettings->Values->GetView()->HasKey(LaunchedLocalSettings))
        {
            compactOptions->CustomSize = Size(320, 394);
            localSettings->Values->Insert(LaunchedLocalSettings, true);
        }
        else
        {
            if (localSettings->Values->GetView()->HasKey(WidthLocalSettings) && localSettings->Values->GetView()->HasKey(HeightLocalSettings))
            {
                float oldWidth = safe_cast<IPropertyValue ^>(localSettings->Values->GetView()->Lookup(WidthLocalSettings))->GetSingle();
                float oldHeight = safe_cast<IPropertyValue ^>(localSettings->Values->GetView()->Lookup(HeightLocalSettings))->GetSingle();
                compactOptions->CustomSize = Size(oldWidth, oldHeight);
            }
            else
            {
                compactOptions->CustomSize = Size(320, 394);
            }
        }

        bool success = co_await ApplicationView::GetForCurrentView()->TryEnterViewModeAsync(ApplicationViewMode::CompactOverlay, compactOptions);
        CalculatorViewModel->AreHistoryShortcutsEnabled = !success;
        CalculatorViewModel->HistoryVM->AreHistoryShortcutsEnabled = !success;
        CalculatorViewModel->IsAlwaysOnTop = success;
        IsAlwaysOnTop = success;
    }
    SetDisplayNormalAlwaysOnTopOption();
};
#pragma optimize("", on)

void ApplicationViewModel::SetDisplayNormalAlwaysOnTopOption()
{
    DisplayNormalAlwaysOnTopOption =
        m_mode == ViewMode::Standard && ApplicationView::GetForCurrentView()->IsViewModeSupported(ApplicationViewMode::CompactOverlay) && !IsAlwaysOnTop;
}
