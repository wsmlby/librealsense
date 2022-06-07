// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2022 Intel Corporation. All Rights Reserved.

#include <types.h>
#include "dds-device-watcher.h"
#include <librealsense2/dds/dds-device.h>
#include <librealsense2/dds/topics/dds-topics.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>


using namespace eprosima::fastdds::dds;
using namespace librealsense;


dds_device_watcher::dds_device_watcher( std::shared_ptr< dds::dds_participant > participant )
    : _subscriber( nullptr )
    , _topic( nullptr )
    , _reader( nullptr )
    , _topic_type( new dds::topics::device_info::type )
    , _init_done( false )
    , _participant( participant )
    , _active_object( [this]( dispatcher::cancellable_timer timer ) {

        if( _reader->wait_for_unread_message( { 1, 0 } ) )
        {
            dds::topics::raw::device_info raw_data;
            SampleInfo info;
            // Process all the samples until no one is returned,
            // We will distinguish info change vs new data by validating using `valid_data` field
            while( ReturnCode_t::RETCODE_OK == _reader->take_next_sample( &raw_data, &info ) )
            {
                // Only samples for which valid_data is true should be accessed
                // valid_data indicates that the instance is still ALIVE and the `take` return an
                // updated sample
                if( info.valid_data )
                {
                    dds::topics::device_info device_info = raw_data;

                    std::lock_guard< std::mutex > lock( _devices_mutex );
                    eprosima::fastrtps::rtps::GUID_t guid;
                    eprosima::fastrtps::rtps::iHandle2GUID( guid, info.publication_handle );

                    LOG_DEBUG( "DDS device (" << guid << ") detected:"
                        << "\n\tName: " << device_info.name
                        << "\n\tSerial: " << device_info.serial
                        << "\n\tProduct line: " << device_info.product_line
                        << "\n\tTopic root: " << device_info.topic_root
                        << "\n\tLocked:" << ( device_info.locked ? "yes" : "no" ) );

                    // Add a new device record into our dds devices map
                    _dds_devices[guid] = dds::dds_device::create( _participant, guid, device_info );

                    // TODO - Call LRS callback to create the RS devices
                    // if( callback )
                    // {
                    //    callback_invocation_holder callback = { _callback_inflight.allocate(),
                    //    &_callback_inflight }; _callback( _devices_data, curr );
                    // }
                        
                }
            }
        }
    } )
{
}

void dds_device_watcher::start( platform::device_changed_callback callback )
{
    stop();
    _callback = std::move( callback );
    if( ! _init_done )
    {
        init();
        _init_done = true;
    }
    _active_object.start();
    LOG_DEBUG( "DDS device watcher started" );
}

void dds_device_watcher::stop()
{
    if( ! is_stopped() )
    {
        _active_object.stop();
        //_callback_inflight.wait_until_empty();
        LOG_DEBUG( "DDS device watcher stopped" );
    }
}


dds_device_watcher::~dds_device_watcher()
{
    stop();

    if( _subscriber != nullptr && _reader != nullptr )
    {
        _subscriber->delete_datareader( _reader );
    }
    if( _participant->is_valid() )
    {
        if( _subscriber != nullptr )
        {
            _participant->get()->delete_subscriber( _subscriber );
        }
        if( _topic != nullptr )
        {
            _participant->get()->delete_topic( _topic );
        }
    }
}

void dds_device_watcher::init()
{
    if( ! _participant->is_valid() )
        throw std::runtime_error( "participant was not initialized" );

    if( ! _listener )
        _participant->create_listener( &_listener )->on_writer_removed( [this]( dds::dds_guid guid ) {
            std::lock_guard< std::mutex > lock( _devices_mutex );
            auto it = _dds_devices.find( guid );
            if( it != _dds_devices.end() )
            {
                auto serial_number = it->second->device_info().serial;
                _dds_devices.erase( it );
                LOG_DEBUG( "DDS device s/n " << serial_number << " removed from domain" );
            }
        } );

    // REGISTER THE TYPE
    _topic_type.register_type( _participant->get() );

    // CREATE THE SUBSCRIBER
    _subscriber = _participant->get()->create_subscriber( SUBSCRIBER_QOS_DEFAULT, nullptr );

    if( _subscriber == nullptr )
    {
        throw librealsense::backend_exception( "Error creating a DDS subscriber",
                                               RS2_EXCEPTION_TYPE_IO );
    }

    // CREATE THE TOPIC
    _topic = _participant->get()->create_topic( librealsense::dds::topics::device_info::TOPIC_NAME,
                                                _topic_type->getName(),
                                                TOPIC_QOS_DEFAULT );

    if( _topic == nullptr )
    {
        throw librealsense::backend_exception( "Error creating a DDS Devices topic",
                                               RS2_EXCEPTION_TYPE_IO );
    }

    // CREATE THE READER
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;

    // The 'depth' parameter of the History defines how many samples are stored before starting to
    // overwrite them with newer ones.
    rqos.history().kind = KEEP_LAST_HISTORY_QOS;
    rqos.history().depth = 10;

    rqos.reliability().kind
        = RELIABLE_RELIABILITY_QOS;  // We don't want to miss connection/disconnection events
    rqos.durability().kind = VOLATILE_DURABILITY_QOS;  // The Subscriber receives samples from the
                                                       // moment it comes online, not before
    rqos.data_sharing().automatic();                   // If possible, use shared memory
    rqos.ownership().kind = EXCLUSIVE_OWNERSHIP_QOS;
    _reader = _subscriber->create_datareader( _topic, rqos, nullptr );

    if( _reader == nullptr )
    {
        throw librealsense::backend_exception( "Error creating a DDS reader",
                                               RS2_EXCEPTION_TYPE_IO );
    }

    LOG_DEBUG( "DDS device watcher initialized successfully" );
}


bool dds_device_watcher::foreach_device(
    std::function< bool( dds::dds_guid const &, std::shared_ptr< dds::dds_device > const & ) > fn )
{
    std::lock_guard< std::mutex > lock( _devices_mutex );
    for( auto && guid_to_dev_info : _dds_devices )
    {
        if( ! fn( guid_to_dev_info.first, guid_to_dev_info.second ) )
            return false;
    }
    return true;
}

