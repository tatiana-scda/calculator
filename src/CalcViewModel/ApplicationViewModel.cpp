// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

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

ApplicationViewModel::ApplicationViewModel()
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
    if (m_mode != value)
    {
        PreviousMode = m_mode;
        m_mode = value;
        SetDisplayNormalAlwaysOnTopOption();
        OnModeChanged();
        RaisePropertyChanged(ModePropertyName);
    }
}

void ApplicationViewModel::Categories::set(IObservableVector<NavCategoryGroup ^> ^ value)
{
    if (m_categories != value)
    {
        m_categories = value;
        RaisePropertyChanged(CategoriesPropertyName);
    }
}

void ApplicationViewModel::Initialize(ViewMode mode)
{
    if (!NavCategory::IsValidViewMode(mode))
    {
        mode = ViewMode::Standard;
    }

    try
    {
        Mode = mode;
    }
    catch (const std::exception& e)
    {
        TraceLogger::GetInstance().LogStandardException(mode, __FUNCTIONW__, e);
        if (!TryRecoverFromNavigationModeFailure())
        {
            throw;
        }
    }
    catch (Exception ^ e)
    {
        TraceLogger::GetInstance().LogPlatformException(mode, __FUNCTIONW__, e);
        if (!TryRecoverFromNavigationModeFailure())
        {
            throw;
        }
    }
}

bool ApplicationViewModel::TryRecoverFromNavigationModeFailure()
{
    try
    {
        Mode = ViewMode::Standard;
        return true;
    }
    catch (...)
    {
        return false;
    }
}

void ApplicationViewModel::OnModeChanged()
{
    assert(NavCategory::IsValidViewMode(m_mode));
    if (NavCategory::IsCalculatorViewMode(m_mode))
    {
        if (!m_CalculatorViewModel)
        {
            m_CalculatorViewModel = ref new StandardCalculatorViewModel();
        }
        m_CalculatorViewModel->SetCalculatorType(m_mode);
    }
    else if (NavCategory::IsDateCalculatorViewMode(m_mode))
    {
        if (!m_DateCalcViewModel)
        {
            m_DateCalcViewModel = ref new DateCalculatorViewModel();
        }
    }
    else if (NavCategory::IsConverterViewMode(m_mode))
    {
        if (!m_ConverterViewModel)
        {
            auto dataLoader = make_shared<UnitConverterDataLoader>(ref new GeographicRegion());
            auto currencyDataLoader = make_shared<CurrencyDataLoader>(make_unique<CurrencyHttpClient>());
            m_ConverterViewModel = ref new UnitConverterViewModel(make_shared<UnitConversionManager::UnitConverter>(dataLoader, currencyDataLoader));
        }

        m_ConverterViewModel->Mode = m_mode;
    }

    auto resProvider = AppResourceProvider::GetInstance();
    CategoryName = resProvider.GetResourceString(NavCategory::GetNameResourceKey(m_mode));

    ApplicationData::Current->LocalSettings->Values->Insert(ModePropertyName, NavCategory::Serialize(m_mode));

    if (NavCategory::IsValidViewMode(m_PreviousMode))
    {
        TraceLogger::GetInstance().LogModeChange(m_mode);
    }
    else
    {
        TraceLogger::GetInstance().LogWindowCreated(m_mode, ApplicationView::GetApplicationViewIdForWindow(CoreWindow::GetForCurrentThread()));
    }

    RaisePropertyChanged(ClearMemoryVisibilityPropertyName);
}

void ApplicationViewModel::OnCopyCommand(Object ^ parameter)
{
    if (NavCategory::IsConverterViewMode(m_mode))
    {
        ConverterViewModel->OnCopyCommand(parameter);
    }
    else if (NavCategory::IsDateCalculatorViewMode(m_mode))
    {
        DateCalcViewModel->OnCopyCommand(parameter);
    }
    else
    {
        CalculatorViewModel->OnCopyCommand(parameter);
    }
}

void ApplicationViewModel::OnPasteCommand(Object ^ parameter)
{
    if (NavCategory::IsConverterViewMode(m_mode))
    {
        ConverterViewModel->OnPasteCommand(parameter);
    }
    else if (NavCategory::IsCalculatorViewMode(m_mode))
    {
        CalculatorViewModel->OnPasteCommand(parameter);
    }
}

void ApplicationViewModel::SetMenuCategories()
{
    Categories = NavCategoryGroup::CreateMenuOptions();
}




