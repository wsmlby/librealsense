// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2022 Intel Corporation. All Rights Reserved.

#pragma once
#include <librealsense2/rs.hpp>
#include <librealsense2/utilities/time/stopwatch.h>

namespace rs2
{
    struct notifications_model;
    class subdevice_model;

    class option_model
    {
    public:
        bool draw( std::string& error_message, notifications_model& model, bool new_line = true, bool use_option_name = true );
        void update_supported( std::string& error_message );
        void update_read_only_status( std::string& error_message );
        void update_all_fields( std::string& error_message, notifications_model& model );
        bool set_option( rs2_option opt,
            float value,
            std::string& error_message,
            std::chrono::steady_clock::duration ignore_period = std::chrono::seconds( 0 ) );
        bool draw_option( bool update_read_only_options, bool is_streaming,
            std::string& error_message, notifications_model& model );

        rs2_option opt;
        option_range range;
        std::shared_ptr<options> endpoint;
        float unset_value = 0;
        bool have_unset_value = false;
        utilities::time::stopwatch last_set_stopwatch;
        bool* invalidate_flag = nullptr;
        bool supported = false;
        bool read_only = false;
        float value = 0.0f;
        std::string label = "";
        std::string id = "";
        subdevice_model* dev;
        std::function<bool( option_model&, std::string&, notifications_model& )> custom_draw_method = nullptr;
        bool edit_mode = false;
        std::string edit_value = "";
    private:
        bool is_all_integers() const;
        bool is_enum() const;
        bool is_checkbox() const;
        bool draw_checkbox( notifications_model& model, std::string& error_message, const char* description );
        bool draw_combobox( notifications_model& model, std::string& error_message, const char* description, bool new_line, bool use_option_name );
        bool draw_slider( notifications_model& model, std::string& error_message, const char* description, bool use_cm_units );
        bool slider_selected( rs2_option opt,
            float value,
            std::string& error_message,
            notifications_model& model );

        bool slider_unselected( rs2_option opt,
            float value,
            std::string& error_message,
            notifications_model& model );
    };
}
