// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2022 Intel Corporation. All Rights Reserved.

/*!
 * @file notificationTypeObject.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifndef _FAST_DDS_GENERATED_LIBREALSENSE_DDS_TOPICS_RAW_DEVICE_NOTIFICATION_TYPE_OBJECT_H_
#define _FAST_DDS_GENERATED_LIBREALSENSE_DDS_TOPICS_RAW_DEVICE_NOTIFICATION_TYPE_OBJECT_H_


#include <fastrtps/types/TypeObject.h>
#include <map>

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif
#else
#define eProsima_user_DllExport
#endif

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(notification_SOURCE)
#define notification_DllAPI __declspec( dllexport )
#else
#define notification_DllAPI __declspec( dllimport )
#endif // notification_SOURCE
#else
#define notification_DllAPI
#endif
#else
#define notification_DllAPI
#endif // _WIN32

using namespace eprosima::fastrtps::types;

eProsima_user_DllExport void registernotificationTypes();

namespace librealsense {
    namespace dds {
        namespace topics {
            namespace raw {
                namespace device {
                    eProsima_user_DllExport const TypeIdentifier* GetnotificationIdentifier(bool complete = false);
                    eProsima_user_DllExport const TypeObject* GetnotificationObject(bool complete = false);
                    eProsima_user_DllExport const TypeObject* GetMinimalnotificationObject();
                    eProsima_user_DllExport const TypeObject* GetCompletenotificationObject();

                } // namespace device
            } // namespace raw
        } // namespace topics
    } // namespace dds
} // namespace librealsense

#endif // _FAST_DDS_GENERATED_LIBREALSENSE_DDS_TOPICS_RAW_DEVICE_NOTIFICATION_TYPE_OBJECT_H_