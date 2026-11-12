# Copyright 2021-2026 Nikita Fediuchin. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

function(stripExecutable STRIP_TARGET)
	if(LINUX)
		add_custom_command(TARGET ${STRIP_TARGET} POST_BUILD VERBATIM
			COMMAND ${CMAKE_COMMAND} -E echo 
				"Stripping $<TARGET_FILE_NAME:${STRIP_TARGET}> executable..."
			COMMAND objcopy --only-keep-debug "$<TARGET_FILE:${STRIP_TARGET}>" 
				"$<TARGET_FILE:${STRIP_TARGET}>.debug"
			COMMAND strip --strip-all "$<TARGET_FILE:${STRIP_TARGET}>")
		set_property(TARGET ${STRIP_TARGET} APPEND PROPERTY 
			ADDITIONAL_CLEAN_FILES "$<TARGET_FILE:${STRIP_TARGET}>.debug")
	elseif(APPLE)
		add_custom_command(TARGET ${STRIP_TARGET} POST_BUILD VERBATIM
			COMMAND ${CMAKE_COMMAND} -E echo 
				"Stripping $<TARGET_FILE_NAME:${STRIP_TARGET}> executable..."
			COMMAND dsymutil "$<TARGET_FILE:${STRIP_TARGET}>"
			COMMAND strip "$<TARGET_FILE:${STRIP_TARGET}>")
	endif()
endfunction()