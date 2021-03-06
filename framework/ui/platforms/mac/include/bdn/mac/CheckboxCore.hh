#ifndef BDN_MAC_CheckboxCore_HH
#define BDN_MAC_CheckboxCore_HH

#include <bdn/ICheckboxCore.h>
#include <bdn/ISwitchCore.h>
#include <bdn/Checkbox.h>
#include <bdn/Toggle.h>

#import <bdn/mac/util.hh>
#import <bdn/mac/ToggleCoreBase.hh>

namespace bdn
{
    namespace mac
    {

        template <class T>
        class CheckboxCore : public ToggleCoreBase, BDN_IMPLEMENTS ICheckboxCore, BDN_IMPLEMENTS ISwitchCore
        {
          public:
            CheckboxCore(T *outer) : ToggleCoreBase(outer)
            {
                if (tryCast<Checkbox>(outer))
                    _nsButton.allowsMixedState = true;

                setLabel(outer->label());
                setState(outer->state());
            }

            // Called when attached to a Checkbox or Toggle with checkbox
            // appearance
            void setState(const TriState &state) override
            {
                if (state == TriState::mixed) {
                    // Explicitly allow for mixed state in NSButton when setting
                    // Checkbox::State::mixed programatically
                    _nsButton.allowsMixedState = true;
                }

                _nsButton.state = triStateToNSControlStateValue(state);
            }

            // Called when attached to a Switch or Toggle with switch appearance
            void setOn(const bool &on) override { setState(on ? TriState::on : TriState::off); }

            void generateClick() override
            {
                P<T> outer = cast<T>(getOuterViewIfStillAttached());
                if (outer != nullptr) {
                    P<Checkbox> checkbox = tryCast<Checkbox>(outer);
                    P<Toggle> toggle = tryCast<Toggle>(outer);

                    bdn::ClickEvent evt(outer);

                    // Observing NSButton's state via KVO does not work when
                    // the button's state is changed via user interaction. KVO
                    // works though when state is set programatically, which
                    // unfortunately is useless in the case that a user changes
                    // the button's state. This means we have to stick to the
                    // click event to propagate the state change to the
                    // framework. The state will be set before the onClick
                    // notification is posted.

                    // Mixed state is supported if T == Checkbox
                    if (checkbox) {
                        // Prohibit setting mixed state via user interaction.
                        // When NSButton allows for mixed state, it also allows
                        // users to toggle the checkbox to mixed state by
                        // clicking (Off => Mixed => On). To prevent this
                        // behavior, we deactivate allowsMixedState when
                        // NSButton is switched to off state and reactivate it
                        // when it is set to on state.
                        if (_nsButton.state == NSControlStateValueOff) {
                            _nsButton.allowsMixedState = false;
                        } else {
                            _nsButton.allowsMixedState = true;
                        }
                    }

                    TriState newState = nsControlStateValueToTriState(_nsButton.state);

                    if (checkbox) {
                        checkbox->setState(newState);
                    } else {
                        toggle->setOn(newState == TriState::on);
                    }
                    outer->onClick().notify(evt);
                }
            }
        };
    }
}

#endif /* BDN_MACCheckboxCore_HH */
