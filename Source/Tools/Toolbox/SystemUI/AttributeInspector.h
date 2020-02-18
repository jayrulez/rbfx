//
// Copyright (c) 2017-2020 the rbfx project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once


#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/SystemUI/SystemUI.h>

#include "ToolboxAPI.h"
#include "SystemUI/Widgets.h"


namespace Urho3D
{

class Viewport;

enum class AttributeInspectorModified : unsigned
{
    NO_CHANGE = 0,
    SET_BY_USER = 1,
    SET_DEFAULT = 1u << 1u,
    SET_INHERITED = 1u << 2u,
    RESET = SET_DEFAULT | SET_INHERITED
};
URHO3D_FLAGSET(AttributeInspectorModified, AttributeInspectorModifiedFlags);

URHO3D_EVENT(E_INSPECTORLOCATERESOURCE, InspectorLocateResource)
{
    URHO3D_PARAM(P_NAME, ResourceName);                                         // String
}

URHO3D_EVENT(E_INSPECTORRENDERSTART, InspectorRenderStart)
{
    URHO3D_PARAM(P_SERIALIZABLE, Serializable);                                 // Serializable*
}

URHO3D_EVENT(E_INSPECTORRENDEREND, InspectorRenderEnd)
{
}

URHO3D_EVENT(E_INSPECTORRENDERATTRIBUTE, InspectorRenderAttribute)
{
    URHO3D_PARAM(P_ATTRIBUTEINFO, AttributeInfo);                               // void*
    URHO3D_PARAM(P_SERIALIZABLE, Serializable);                                 // RefCounted*
    URHO3D_PARAM(P_HANDLED, Handled);                                           // bool
    URHO3D_PARAM(P_MODIFIED, Modified);                                         // unsigned (AttributeInspectorModifiedFlags)
}
/// Automate tracking of initial values that are modified by ImGui widget.
template<typename Value>
struct ValueHistory
{
    using ValueCopy = typename ea::remove_cvref_t<Value>;
    using ValueRef = typename std::conditional_t<!std::is_reference_v<std::remove_cv_t<Value>>, const ValueCopy&, ValueCopy&>;

    ValueHistory(ValueRef current) : current_(current) { }

    static ValueHistory& Get(ValueRef value)
    {
        auto history = ui::GetUIState<ValueHistory>(value);
        if (history->expired_)
        {
            history->initial_ = history->current_ = value;
            history->expired_ = false;
        }
        return *history;
    }

    /// Returns true when value is modified and no continuous modification is happening.
    bool IsModified()
    {
        if (initial_ != current_ && !ui::IsAnyItemActive())
        {
            expired_ = true;
            return true;
        }
        return false;
    }
    /// Returns true if value is modified.
    explicit operator bool() { return IsModified(); }

    /// Initial value.
    ValueCopy initial_{};
    /// Last value.
    Value current_{};
    /// Flag indicating this history entry is expired and initial value can be overwritten.
    bool expired_ = true;

};

/// A dummy object used as namespace for subscribing to events.
class URHO3D_TOOLBOX_API AttributeInspector : public Object
{
    URHO3D_OBJECT(AttributeInspector, Object);
public:
    explicit AttributeInspector(Context* context) : Object(context) { }
};

/// Render attribute inspector of `item`.
/// If `filter` is not null then only attributes containing this substring will be rendered.
/// If `eventNamespace` is not null then this object will be used to send events.
URHO3D_TOOLBOX_API bool RenderAttributes(Serializable* item, const char* filter=nullptr, Object* eventNamespace=nullptr);
URHO3D_TOOLBOX_API bool RenderSingleAttribute(Variant& value);

}
