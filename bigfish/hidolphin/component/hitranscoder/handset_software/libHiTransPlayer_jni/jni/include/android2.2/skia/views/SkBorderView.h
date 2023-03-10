/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SkBorderView_DEFINED
#define SkBorderView_DEFINED

#include "SkView.h"
#include "SkWidgetViews.h"
#include "SkAnimator.h"

class SkBorderView : public SkWidgetView {
public:
    SkBorderView();
    ~SkBorderView();
    void setSkin(const char skin[]);
    SkScalar getLeft() const { return fLeft; }
    SkScalar getRight() const { return fRight; }
    SkScalar getTop() const { return fTop; }
    SkScalar getBottom() const { return fBottom; }
protected:
    //overrides
    virtual void onInflate(const SkDOM& dom,  const SkDOM::Node* node);
    virtual void onSizeChange();
    virtual void onDraw(SkCanvas* canvas);
    virtual bool onEvent(const SkEvent& evt);
private:
    SkAnimator fAnim;
    SkScalar fLeft, fRight, fTop, fBottom;  //margin on each side
    SkRect fMargin;

    typedef SkWidgetView INHERITED;
};

#endif
