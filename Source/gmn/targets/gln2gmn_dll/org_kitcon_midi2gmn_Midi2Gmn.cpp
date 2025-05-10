/*
	midi2gmn Library
	Copyright (C) 1992 - 2012 Juergen F. Kilian


	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#include <jni.h>
                
#include "org_kitcon_midi2gmn_Midi2Gmn.h"
#include <string>
#include <stdio.h>

#include "../../process.h"

// http://dev.eclipse.org/newslists/news.eclipse.tools.cdt/msg17098.html

JNIEXPORT jstring JNICALL Java_org_kitcon_midi2gmn_Midi2Gmn_process
  (JNIEnv *env, jclass obj,  jstring glnString, jstring iniPath)
{
	// for mingw use -framework JavaVM

	const char *glnData_c = env->GetStringUTFChars( glnString, 0);
	const char *iniPath_c = env->GetStringUTFChars( iniPath, 0);

	std::string glnData = std::string(glnData_c);
	std::string iniPath_str = std::string(iniPath_c);

	std::string msg = processString( &glnData,  iniPath_str);

	env->ReleaseStringUTFChars( glnString, glnData_c);
	env->ReleaseStringUTFChars( iniPath, iniPath_c);

	return env->NewStringUTF(msg.c_str());

}



