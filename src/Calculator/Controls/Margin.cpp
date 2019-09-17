
#include "pch.h"
#include "CalculationResult.h"
#include "CalculationResultAutomationPeer.h"
#include "CalcViewModel/Common/LocalizationSettings.h"

using namespace CalculatorApp;
using namespace CalculatorApp::Controls;
using namespace CalculatorApp::Common;

using namespace Platform;
using namespace Windows::Devices::Input;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Automation;
using namespace Windows::UI::Xaml::Automation::Peers;
using namespace std;

DEPENDENCY_PROPERTY_INITIALIZATION(CalculationResult, DisplayMargin);extBox->FontSize = newFontSize;
}

#define SCALEFACTOR 0.357143
#define SMALLHEIGHTSCALEFACTOR 0
#define HEIGHTCUTOFF 100
#define INCREMENTOFFSET 1
#define MAXFONTINCREMENT 5
#define WIDTHTOFONTSCALAR 0.0556513
#define WIDTHTOFONTOFFSET 3
#define WIDTHCUTOFF 50
#define FONTTOLERANCE 0.001

StringReference CalculationResult::s_FocusedState(L"Focused");
StringReference CalculationResult::s_UnfocusedState(L"Unfocused");

void CalculationResult::ModifyFontAndMargin(TextBlock ^ textBox, double fontChange)
{
    double cur = textBox->FontSize;
    double newFontSize = 0.0;
    double scaleFactor = SCALEFACTOR;
    if (m_textContainer->ActualHeight <= HEIGHTCUTOFF)
    {
        scaleFactor = SMALLHEIGHTSCALEFACTOR;
    }

    newFontSize = min(max(cur + fontChange, MinFontSize), MaxFontSize);
    m_textContainer->Padding = Thickness(0, 0, 0, scaleFactor * abs(cur - newFontSize));
    textBox->FontSize = newFontSize;
}

void CalculationResult::UpdateTextState()
{
    if ((m_textContainer == nullptr) || (m_textBlock == nullptr))
    {
        return;
    }

    auto containerSize = m_textContainer->ActualWidth;
    String ^ oldText = m_textBlock->Text;
    String ^ newText = Utils::LRO + DisplayValue + Utils::PDF;

    // Initiate the scaling operation
    // UpdateLayout will keep calling us until we make it through the below 2 if-statements
    if (!m_isScalingText || oldText != newText)
    {
        m_textBlock->Text = newText;

        m_isScalingText = true;
        m_haveCalculatedMax = false;
        m_textBlock->InvalidateArrange();
        return;
    }
    if (containerSize > 0)
    {
        double widthDiff = abs(m_textBlock->ActualWidth - containerSize);
        double fontSizeChange = INCREMENTOFFSET;

        if (widthDiff > WIDTHCUTOFF)
        {
            fontSizeChange = min<double>(max<double>(floor(WIDTHTOFONTSCALAR * widthDiff) - WIDTHTOFONTOFFSET, INCREMENTOFFSET), MAXFONTINCREMENT);
        }
        if (m_textBlock->ActualWidth < containerSize && abs(m_textBlock->FontSize - MaxFontSize) > FONTTOLERANCE && !m_haveCalculatedMax)
        {
            ModifyFontAndMargin(m_textBlock, fontSizeChange);
            m_textBlock->InvalidateArrange();
            return;
        }
        if (fontSizeChange < 5)
        {
            m_haveCalculatedMax = true;
        }
        if (m_textBlock->ActualWidth >= containerSize && abs(m_textBlock->FontSize - MinFontSize) > FONTTOLERANCE)
        {
            ModifyFontAndMargin(m_textBlock, -1 * fontSizeChange);
            m_textBlock->InvalidateArrange();
            return;
        }
        assert(m_textBlock->FontSize >= MinFontSize && m_textBlock->FontSize <= MaxFontSize);
        m_isScalingText = false;
        if (IsOperatorCommand)
        {
            m_textContainer->ChangeView(0.0, nullptr, nullptr);
        }
        else
        {
            m_textContainer->ChangeView(m_textContainer->ExtentWidth - m_textContainer->ViewportWidth, nullptr, nullptr);
        }

        if (m_scrollLeft && m_scrollRight)
        {
            if (m_textBlock->ActualWidth < containerSize)
            {
                ShowHideScrollButtons(::Visibility::Collapsed, ::Visibility::Collapsed);
            }
            else
            {
                if (IsOperatorCommand)
                {
                    ShowHideScrollButtons(::Visibility::Collapsed, ::Visibility::Visible);
                }
                else
                {
                    ShowHideScrollButtons(::Visibility::Visible, ::Visibility::Collapsed);
                }
            }
        }
        m_textBlock->InvalidateArrange();
    }
}
