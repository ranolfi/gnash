// Stage.cpp:  All the world is one, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "Stage.h"
#include "movie_root.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "VM.h"
#include "Object.h" // for getObjectInterface()
#include "AsBroadcaster.h" // for initializing self as a broadcaster
#include "namedStrings.h"
#include "StringPredicates.h"

#include <string>

namespace gnash {

static as_value stage_scalemode_getset(const fn_call& fn);
static as_value stage_align_getset(const fn_call& fn);
static as_value stage_showMenu_getset(const fn_call& fn);
static as_value stage_width_getset(const fn_call& fn);
static as_value stage_height_getset(const fn_call& fn);
static as_value stage_displaystate_getset(const fn_call& fn);
static const char* getScaleModeString(movie_root::ScaleMode sm);
static const char* getDisplayStateString(Stage::DisplayState ds);

void registerStageNative(as_object& o)
{
	VM& vm = o.getVM();
	
	vm.registerNative(stage_scalemode_getset, 666, 1);
    vm.registerNative(stage_scalemode_getset, 666, 2);
    vm.registerNative(stage_align_getset, 666, 3);
	vm.registerNative(stage_align_getset, 666, 4);
	vm.registerNative(stage_width_getset, 666, 5);
    vm.registerNative(stage_width_getset, 666, 6);
	vm.registerNative(stage_height_getset, 666, 7);
    vm.registerNative(stage_height_getset, 666, 8);
	vm.registerNative(stage_showMenu_getset, 666, 9);
    vm.registerNative(stage_showMenu_getset, 666, 10);
}

static void
attachStageInterface(as_object& o)
{
	VM& vm = o.getVM();

    const int version = vm.getSWFVersion();

	if ( version < 5 ) return;

    as_c_function_ptr getset;
    
    getset = stage_scalemode_getset;
	o.init_property("scaleMode", getset, getset);

	// Stage.align getter-setter
    getset = stage_align_getset;
	o.init_property("align", getset, getset);

	// Stage.width getter-setter
    getset = stage_width_getset;
	o.init_property("width", getset, getset);

	// Stage.height getter-setter
    getset = stage_height_getset;
	o.init_property("height", getset, getset);

	// Stage.showMenu getter-setter
    getset = stage_showMenu_getset;
	o.init_property("showMenu", getset, getset);

    getset = stage_displaystate_getset;
	o.init_property("displayState", getset, getset);

}


Stage::Stage()
	:
	as_object(getObjectInterface()),
	_alignMode(""),
	_displayState(normal)
{
	attachStageInterface(*this);

	const int swfversion = _vm.getSWFVersion();
	if ( swfversion > 5 )
	{
		AsBroadcaster::initialize(*this);
	}
}


void
Stage::onResize()
{
	as_value v;
	if (get_member(NSV::PROP_SCALE_MODE, &v) && v.to_string() == "noScale" )
	{
		notifyResize();
	}
}


void
Stage::notifyResize()
{
	log_debug("notifying Stage listeners about a resize");
	callMethod(NSV::PROP_BROADCAST_MESSAGE, "onResize");
}


void
Stage::setDisplayState(DisplayState state)
{
	if ( _displayState == state ) return; // nothing to do

	_displayState = state;
	
	if (!movie_root::interfaceHandle) return; // No registered callback
	
	if (_displayState == Stage::fullScreen)
	{
	    (*movie_root::interfaceHandle)("Stage.displayState", "fullScreen");
	}
	else if (_displayState == Stage::normal)
	{
	    (*movie_root::interfaceHandle)("Stage.displayState", "normal");
	}

}


void
Stage::setAlignMode(const std::string& mode)
{
    if (_alignMode == mode) return;

    _alignMode = mode;
    
    movie_root::StageVerticalAlign v = movie_root::STAGE_V_ALIGN_C;
    movie_root::StageHorizontalAlign h = movie_root::STAGE_H_ALIGN_C;

    // Order: LTRB. L and T take precedence.

    for (std::string::const_reverse_iterator it = _alignMode.rbegin(),
         e = _alignMode.rend();
         it != e;
         ++it)
    {
        switch (*it)
        {
            case 'R':
                h = movie_root::STAGE_H_ALIGN_R;
                break;
            case 'L':
                h = movie_root::STAGE_H_ALIGN_L;
                break;
            case 'B':
                v = movie_root::STAGE_V_ALIGN_B;
                break;
            case 'T':
                v = movie_root::STAGE_V_ALIGN_T;
                break;
        }
    }

    movie_root& m = VM::get().getRoot();
    m.setStageAlignment(h, v);   

}


const char*
getDisplayStateString(Stage::DisplayState ds)
{
	static const char* displayStateName[] = {
		"normal",
		"fullScreen" };

	return displayStateName[ds];
}


const char*
getScaleModeString(movie_root::ScaleMode sm)
{
	static const char* modeName[] = {
		"showAll",
		"noScale",
		"exactFit",
		"noBorder" };

	return modeName[sm];
}


as_value
stage_scalemode_getset(const fn_call& fn)
{
    const movie_root& m = VM::get().getRoot();

	if ( fn.nargs == 0 ) // getter
	{
		return as_value(getScaleModeString(m.getScaleMode()));
	}
	else // setter
	{
		movie_root::ScaleMode mode = movie_root::showAll;

		const std::string& str = fn.arg(0).to_string();
		
		StringNoCaseEqual noCaseCompare;
		
		if ( noCaseCompare(str, "noScale") ) mode = movie_root::noScale;
		else if ( noCaseCompare(str, "exactFit") ) mode = movie_root::exactFit;
		else if ( noCaseCompare(str, "noBorder") ) mode = movie_root::noBorder;

        movie_root& m = VM::get().getRoot();

        if ( m.getScaleMode() == mode ) return as_value(); // nothing to do

	    m.setScaleMode(mode);
		return as_value();
	}
}

as_value
stage_width_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Stage> stage = ensureType<Stage>(fn.this_ptr);
	UNUSED(stage);

	if ( fn.nargs > 0 ) // setter
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Stage.width is a read-only property!"));
		);
		return as_value();
	}

    // getter
    movie_root& m = VM::get().getRoot();
    return as_value(m.getStageWidth());
}

as_value
stage_height_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Stage> stage = ensureType<Stage>(fn.this_ptr);
	UNUSED(stage);

	if ( fn.nargs > 0 ) // setter
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Stage.height is a read-only property!"));
		);
		return as_value();
	}

    // getter
    movie_root& m = VM::get().getRoot();
    return as_value(m.getStageHeight());
}


as_value
stage_align_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Stage> stage = ensureType<Stage>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
	    return as_value (stage->getAlignMode());
	}
	else // setter
	{

		const std::string& str = fn.arg(0).to_string();
        std::string alignMode;

        if (str.find_first_of("lL") != std::string::npos)
        {
            alignMode.push_back('L');
        } 

        if (str.find_first_of("tT") != std::string::npos)
        {
            alignMode.push_back('T');
        } 

        if (str.find_first_of("rR") != std::string::npos)
        {
            alignMode.push_back('R');
        } 
    
        if (str.find_first_of("bB") != std::string::npos)
        {
            alignMode.push_back('B');
        }

        stage->setAlignMode(alignMode);

		return as_value();
	}
}

as_value
stage_showMenu_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Stage> stage = ensureType<Stage>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		static bool warned = false;
		if ( ! warned ) {
			log_unimpl("Stage.showMenu getter");
			warned=true;
		}
		return as_value();
	}
	else // setter
	{
		static bool warned = false;
		if ( ! warned ) {
			log_unimpl("Stage.showMenu setter");
			warned=true;
		}
		return as_value();
	}
}

as_value
stage_displaystate_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Stage> stage = ensureType<Stage>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		return as_value(getDisplayStateString(stage->getDisplayState()));
	}
	else // setter
	{

        StringNoCaseEqual noCaseCompare;

		const std::string& str = fn.arg(0).to_string();
		if ( noCaseCompare(str, "normal") )
		{
		    stage->setDisplayState(Stage::normal);
		}
		else if ( noCaseCompare(str, "fullScreen") ) 
		{
		    stage->setDisplayState(Stage::fullScreen);
        }

        // If invalid, do nothing.
		return as_value();
	}
}

// extern (used by Global.cpp)
void stage_class_init(as_object& global)
{
	static boost::intrusive_ptr<as_object> obj = new Stage();
	global.init_member("Stage", obj.get());
}

} // end of gnash namespace
