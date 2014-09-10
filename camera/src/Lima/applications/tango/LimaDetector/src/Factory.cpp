#include <Factory.h>

#include <iostream>

bool  ControlFactory::is_created = false;


//-----------------------------------------------------------------------------------------
void ControlFactory::initialize_pointers()
{
    my_control				= 0;

#ifdef SIMULATOR_ENABLED
    my_camera_simulator 	= 0;
    my_interface_simulator 	= 0;
#endif

#ifdef AVIEX_ENABLED
    my_camera_aviex 		= 0;
    my_interface_aviex   	= 0;
#endif
    
#ifdef BASLER_ENABLED
    my_camera_basler 		= 0;
    my_interface_basler 	= 0;
#endif

#ifdef XPAD_ENABLED
    my_camera_xpad 			= 0;
    my_interface_xpad 		= 0;
#endif

#ifdef PILATUS_ENABLED
    my_camera_pilatus 		= 0;
    my_interface_pilatus 	= 0;
#endif

#ifdef MARCCD_ENABLED
    my_camera_marccd 		= 0;
    my_interface_marccd 	= 0;
#endif

#ifdef ADSC_ENABLED
    my_camera_adsc 			= 0;
    my_interface_adsc 		= 0;
#endif

#ifdef PROSILICA_ENABLED
    my_camera_prosilica 	= 0;
    my_interface_prosilica 	= 0;
#endif

#ifdef PRINCETON_ENABLED
    my_camera_princeton 	= 0;
    my_interface_princeton 	= 0;
#endif

#ifdef PCO_ENABLED
    my_camera_pco 	        = 0;
    my_interface_pco 	    = 0;
#endif

#ifdef PERKINELMER_ENABLED
    my_interface_perkinelmer= 0;
#endif

#ifdef ANDOR3_ENABLED
    my_camera_andor3        = 0;
    my_interface_andor3	    = 0;
#endif

#ifdef VIEWORKSVP_ENABLED
    my_camera_vieworksvp    = 0;
    my_interface_vieworksvp = 0;
#endif

    my_server_name 			= "none";
    my_device_name 			= "none";

    my_status.str("");
    my_state 				= Tango::INIT;
}

//-----------------------------------------------------------------------------------------
CtControl* ControlFactory::get_control( const std::string& detector_type)
{
    yat::MutexLock scoped_lock(object_lock);
    try
    {
        //get the tango device/instance
        if(!ControlFactory::is_created)
        {
			initialize_pointers();
            std::string  detector = detector_type;
            Tango::DbDatum db_datum;
            my_server_name = Tango::Util::instance()->get_ds_name ();
            db_datum = (Tango::Util::instance()->get_database())->get_device_name(my_server_name,detector);
            db_datum >> my_device_name;
        }

#ifdef SIMULATOR_ENABLED
        if (detector_type == "SimulatorCCD")
        {
            if(!ControlFactory::is_created)
            {       
                my_camera_simulator         = new lima::Simulator::Camera();
                my_interface_simulator      = new lima::Simulator::Interface(*my_camera_simulator);
                my_control                  = new lima::CtControl(my_interface_simulator);
                ControlFactory::is_created  = true;
                return my_control;
            }
        }
#endif

#ifdef AVIEX_ENABLED
        if (detector_type == "AviexCCD")
        {
            if(!ControlFactory::is_created)
            {
                Tango::DbData db_data;
                db_data.push_back(Tango::DbDatum("DetectorID"));
                db_data.push_back(Tango::DbDatum("MxDatabaseFileFullName"));
                (Tango::Util::instance()->get_database())->get_device_property(my_device_name, db_data);
                std::string detector_id;
                std::string database_file;
                db_data[0] >> detector_id;
                db_data[1] >> database_file;
                my_camera_aviex            = new lima::Aviex::Camera(detector_id, database_file);                
                my_interface_aviex         = new lima::Aviex::Interface(*my_camera_aviex);
                my_control                 = new lima::CtControl(my_interface_aviex);          
                ControlFactory::is_created = true;
                return my_control;
            }
        }
#endif
        
#ifdef BASLER_ENABLED
        if (detector_type == "BaslerCCD")
        {
            if(!ControlFactory::is_created)
            {
                Tango::DbData db_data;
                db_data.push_back(Tango::DbDatum("DetectorIP"));
                db_data.push_back(Tango::DbDatum("DetectorTimeout"));
                db_data.push_back(Tango::DbDatum("DetectorPacketSize"));
                (Tango::Util::instance()->get_database())->get_device_property(my_device_name, db_data);
                std::string camera_ip;
                db_data[0] >> camera_ip;
                long detector_timeout = 11000;
                db_data[1] >> detector_timeout;                
                long packet_size = -1;
                db_data[2] >> packet_size;
                my_camera_basler            = new lima::Basler::Camera(camera_ip, packet_size);                
                my_camera_basler->setTimeout(detector_timeout);      
                my_interface_basler         = new lima::Basler::Interface(*my_camera_basler);
                my_control                  = new lima::CtControl(my_interface_basler);          
                ControlFactory::is_created  = true;
                return my_control;
            }
        }
#endif

#ifdef XPAD_ENABLED
        if (detector_type == "XpadPixelDetector")
        {    

            if(!ControlFactory::is_created)
            {
                Tango::DbData db_data;
                db_data.push_back(Tango::DbDatum("XpadModel"));
                db_data.push_back(Tango::DbDatum("CalibrationAdjustingNumber"));
                (Tango::Util::instance()->get_database())->get_device_property(my_device_name, db_data);
                std::string xpad_model;
                Tango::DevULong calibration_adjusting_number;

                db_data[0] >> xpad_model;
                db_data[1] >> calibration_adjusting_number;

                my_camera_xpad                = new lima::Xpad::Camera(xpad_model);
                my_camera_xpad->setCalibrationAdjustingNumber(calibration_adjusting_number);

                my_interface_xpad             = new lima::Xpad::Interface(*my_camera_xpad);
                my_control                    = new lima::CtControl(my_interface_xpad);
                ControlFactory::is_created    = true;
                return my_control;
            }
        }
#endif

#ifdef PILATUS_ENABLED
        if (detector_type == "PilatusPixelDetector")
        {

            if(!ControlFactory::is_created)
            {
                Tango::DbData db_data;
                db_data.push_back(Tango::DbDatum("DetectorIP"));
                db_data.push_back(Tango::DbDatum("DetectorPort"));
                db_data.push_back(Tango::DbDatum("UseReader"));
                (Tango::Util::instance()->get_database())->get_device_property(my_device_name, db_data);
                std::string camera_ip;
                long camera_port;
                bool use_reader;
                db_data[0] >> camera_ip;
                db_data[1] >> camera_port;
                db_data[2] >> use_reader;

                my_camera_pilatus           = new lima::Pilatus::Camera(camera_ip.c_str(), camera_port);
                if(my_camera_pilatus && use_reader)
                    my_camera_pilatus->enableDirectoryWatcher();
                if(my_camera_pilatus && !use_reader)
                    my_camera_pilatus->disableDirectoryWatcher();
                my_interface_pilatus        = new lima::Pilatus::Interface(*my_camera_pilatus);
                my_control                  = new lima::CtControl(my_interface_pilatus);
                ControlFactory::is_created  = true;
                return my_control;
            }
        }
#endif

#ifdef MARCCD_ENABLED
        if (detector_type == "MarCCD")
        {
            if(!ControlFactory::is_created)
            {
                Tango::DbData db_data;
                db_data.push_back(Tango::DbDatum("DetectorIP"));
                db_data.push_back(Tango::DbDatum("DetectorPort"));
                db_data.push_back(Tango::DbDatum("DetectorTargetPath"));
                db_data.push_back(Tango::DbDatum("ReaderTimeout"));                

                (Tango::Util::instance()->get_database())->get_device_property(my_device_name, db_data);
                std::string camera_ip;
                std::string img_path;
                unsigned long camera_port = 2222;
                unsigned short reader_timeout = 10000;

                db_data[0] >> camera_ip;
                db_data[1] >> camera_port;
                db_data[2] >> img_path;
                db_data[3] >> reader_timeout;

                my_camera_marccd           = new lima::Marccd::Camera(camera_ip.c_str(), camera_port, img_path);
                my_interface_marccd        = new lima::Marccd::Interface(*my_camera_marccd);
                if(my_interface_marccd)
                    my_interface_marccd->setTimeout(reader_timeout/1000);
                my_control                 = new lima::CtControl(my_interface_marccd);
                ControlFactory::is_created = true;
                return my_control;
            }
        }
#endif

#ifdef ADSC_ENABLED
        if (detector_type == "AdscCCD")
        {

            if(!ControlFactory::is_created)
            {
                Tango::DbData db_data;
                db_data.push_back(Tango::DbDatum("ReaderTimeout"));
                db_data.push_back(Tango::DbDatum("UseReader"));
                (Tango::Util::instance()->get_database())->get_device_property(my_device_name, db_data);
                short reader_timeout = 1000;
                bool use_reader;
                db_data[0] >> reader_timeout;
                db_data[1] >> use_reader;
                my_camera_adsc                = new lima::Adsc::Camera();
                my_interface_adsc             = new lima::Adsc::Interface(*my_camera_adsc);
                if(my_interface_adsc && use_reader)
                    my_interface_adsc->enableReader();
                if(my_interface_adsc && !use_reader)
                    my_interface_adsc->disableReader();
                if(my_interface_adsc)
                    my_interface_adsc->setTimeout(reader_timeout);
                my_control                    = new lima::CtControl(my_interface_adsc);
                ControlFactory::is_created    = true;
                return my_control;
            }
        }
#endif        

#ifdef PROSILICA_ENABLED
        if (detector_type == "ProsilicaCCD")
        {

            if(!ControlFactory::is_created)
            {
                Tango::DbData db_data;
                db_data.push_back(Tango::DbDatum("DetectorIP"));
                (Tango::Util::instance()->get_database())->get_device_property(my_device_name, db_data);
                std::string camera_ip;
                db_data[0] >> camera_ip;

                my_camera_prosilica           	= new lima::Prosilica::Camera(camera_ip.c_str());
                my_interface_prosilica        	= new lima::Prosilica::Interface(my_camera_prosilica);
                my_control                  	= new lima::CtControl(my_interface_prosilica);
                ControlFactory::is_created  	= true;
                return my_control;
            }
        }
#endif

#ifdef PRINCETON_ENABLED
        if (detector_type == "PrincetonCCD")
        {

            if(!ControlFactory::is_created)
            {
                Tango::DbData db_data;
				db_data.push_back(Tango::DbDatum("DetectorNum"));
                (Tango::Util::instance()->get_database())->get_device_property(my_device_name, db_data);
                long camera_num;
                db_data[0] >> camera_num;
                my_camera_princeton           	= new lima::RoperScientific::Camera(camera_num);
                my_interface_princeton        	= new lima::RoperScientific::Interface(*my_camera_princeton);
                my_control                  	= new lima::CtControl(my_interface_princeton);
                ControlFactory::is_created  	= true;
                return my_control;
            }
        }
#endif

#ifdef PCO_ENABLED
        if (detector_type == "Pco")
        {
            if(!ControlFactory::is_created)
            {
                my_camera_pco               = new lima::Pco::Camera("");
                my_interface_pco            = new lima::Pco::Interface(my_camera_pco);
                my_control                  = new lima::CtControl(my_interface_pco);
                ControlFactory::is_created  = true;
                return my_control;
            }
        }
#endif

#ifdef PERKINELMER_ENABLED
        if (detector_type == "PerkinElmer")
        {
            if(!ControlFactory::is_created)
            {
                my_interface_perkinelmer    = new lima::PerkinElmer::Interface();
                my_control                  = new lima::CtControl(my_interface_perkinelmer);
                ControlFactory::is_created  = true;
                return my_control;
            }
        }
#endif

#ifdef ANDOR3_ENABLED
        if (detector_type == "Andor3")
        {
            if(!ControlFactory::is_created)
            {
                Tango::DbData db_data;
				db_data.push_back(Tango::DbDatum("BitFlowPath"));
                db_data.push_back(Tango::DbDatum("CameraNumber"));
                (Tango::Util::instance()->get_database())->get_device_property(my_device_name, db_data);
                std::string bit_flow_path;
                long camera_number;
                db_data[0] >> bit_flow_path;
                db_data[1] >> camera_number;
                
                my_camera_andor3            = new lima::Andor3::Camera(bit_flow_path,camera_number);
                my_interface_andor3         = new lima::Andor3::Interface(*my_camera_andor3);
                my_control                  = new lima::CtControl(my_interface_andor3);
                ControlFactory::is_created  = true;
                return my_control;
            }
        }
#endif

#ifdef VIEWORKSVP_ENABLED
        if (detector_type == "VieworksVP")
        {
            if(!ControlFactory::is_created)
            {
                Tango::DbData db_data;
                db_data.push_back(Tango::DbDatum("SisoPath"));
				db_data.push_back(Tango::DbDatum("BoardIndex"));
                db_data.push_back(Tango::DbDatum("CameraPort"));
                db_data.push_back(Tango::DbDatum("AppletName"));
                db_data.push_back(Tango::DbDatum("DMAIndex"));
                
                (Tango::Util::instance()->get_database())->get_device_property(my_device_name, db_data);
                std::string siso_path;
                long board_index;
                long camera_port;
                std::string applet_name;
                unsigned long dma_index;
                db_data[0] >> siso_path;
                db_data[1] >> board_index;
                db_data[2] >> camera_port;
                db_data[3] >> applet_name;
                db_data[4] >> dma_index;
                
                my_camera_vieworksvp        = new lima::VieworksVP::Camera(siso_path,board_index,camera_port,applet_name,dma_index);
                my_interface_vieworksvp     = new lima::VieworksVP::Interface(*my_camera_vieworksvp);
                my_control                  = new lima::CtControl(my_interface_vieworksvp);
                ControlFactory::is_created  = true;
                return my_control;
            }
        }
#endif

        

        if(!ControlFactory::is_created)
            throw LIMA_HW_EXC(Error, "Unable to create the lima control object : Unknown Detector Type");

    }
    catch(Tango::DevFailed& df)
    {
        //- rethrow exception
        throw LIMA_HW_EXC(Error, std::string(df.errors[0].desc).c_str());
    }
    catch(Exception& e)
    {
        throw LIMA_HW_EXC(Error, e.getErrMsg());
    }
    catch(...)
    {
        throw LIMA_HW_EXC(Error, "Unable to create the lima control object : Unknown Exception");
    }
    return my_control;
}


//-----------------------------------------------------------------------------------------
void ControlFactory::reset(const std::string& detector_type )
{
    yat::MutexLock scoped_lock(object_lock);
    try
    {
        if(ControlFactory::is_created)
        {
            ControlFactory::is_created = false;
            if(my_control)
            {
                delete my_control;
                my_control = 0;
            }

#ifdef SIMULATOR_ENABLED
            if (detector_type == "SimulatorCCD")
            {
                if(my_camera_simulator)
                {
                    delete my_camera_simulator;
                    my_camera_simulator = 0;
                }

                if(my_interface_simulator)
                {
                    delete my_interface_simulator;
                    my_interface_simulator = 0;
                }
            }
#endif        

#ifdef AVIEX_ENABLED
            if (detector_type == "AviexCCD")
            {
                if(my_camera_aviex)
                {
                    delete my_camera_aviex;
                    my_camera_aviex = 0;
                }

                if(my_interface_aviex)
                {
                    delete my_interface_aviex;
                    my_interface_aviex = 0;
                }
            }
#endif
            
#ifdef BASLER_ENABLED
            if (detector_type == "BaslerCCD")
            {
                if(my_camera_basler)
                {
                    delete my_camera_basler;
                    my_camera_basler = 0;
                }

                if(my_interface_basler)
                {
                    delete my_interface_basler;
                    my_interface_basler = 0;
                }
            }
#endif

#ifdef XPAD_ENABLED
            if (detector_type == "XpadPixelDetector")
            {
                if(my_camera_xpad)
                {
                    //- do not delete because its a YAT Task
                    my_camera_xpad->exit();
                    my_camera_xpad = 0;
                }

                if(my_interface_xpad)
                {
                    delete my_interface_xpad;
                    my_interface_xpad = 0;
                }
            }
#endif

#ifdef PILATUS_ENABLED
            if (detector_type == "PilatusPixelDetector")
            {
                if(my_camera_pilatus)
                {
                    delete my_camera_pilatus;
                    my_camera_pilatus = 0;
                }

                if(my_interface_pilatus)
                {
                    delete my_interface_pilatus;
                    my_interface_pilatus = 0;
                }
            }
#endif

#ifdef MARCCD_ENABLED
            if (detector_type == "MarCCD")
            {
                if(my_camera_marccd)
                {
                    //- do not delete because its a YAT Task
                    my_camera_marccd->exit();
                    my_camera_marccd = 0;
                }

                if(my_interface_marccd)
                {
                    delete my_interface_marccd;
                    my_interface_marccd = 0;
                }
            }
#endif     

#ifdef ADSC_ENABLED
            if (detector_type == "AdscCCD")
            {
                if(my_camera_adsc)
                {
                    delete my_camera_adsc;
                    my_camera_adsc = 0;
                }

                if(my_interface_adsc)
                {
                    delete my_interface_adsc;
                    my_interface_adsc = 0;
                }
            }
#endif

#ifdef PROSILICA_ENABLED
            if (detector_type == "ProsilicaCCD")
            {
                if(my_camera_prosilica)
                {
                    delete my_camera_prosilica;
                    my_camera_prosilica = 0;
                }

                if(my_interface_prosilica)
                {
                    delete my_interface_prosilica;
                    my_interface_prosilica = 0;
                }
            }
#endif

#ifdef PRINCETON_ENABLED
            if (detector_type == "PrincetonCCD")
            {
                if(my_interface_princeton)
                {
                    delete my_interface_princeton;
                    my_interface_princeton = 0;
                }

                if(my_camera_princeton)
                {
                    delete my_camera_princeton;
                    my_camera_princeton = 0;
                }
            }
#endif

#ifdef PCO_ENABLED
            if (detector_type == "Pco")
            {
                if(my_interface_pco)
                {
                    delete my_interface_pco;
                    my_interface_pco = 0;
                }

                if(my_camera_pco)
                {
                    delete my_camera_pco;
                    my_camera_pco = 0;
                }
            }
#endif

#ifdef PERKINELMER_ENABLED
            if (detector_type == "PerkinElmer")
            {
                if(my_interface_perkinelmer)
                {
                    delete my_interface_perkinelmer;
                    my_interface_perkinelmer = 0;
                }                
            }
#endif

#ifdef ANDOR3_ENABLED
            if (detector_type == "Andor3")
            {
                if(my_interface_andor3)
                {
                    delete my_interface_andor3;
                    my_interface_andor3 = 0;
                }

                if(my_camera_andor3)
                {
                    delete my_camera_andor3;
                    my_camera_andor3 = 0;
                }
            }
#endif

#ifdef VIEWORKSVP_ENABLED
            if (detector_type == "VieworksVP")
            {
                if(my_interface_vieworksvp)
                {
                    delete my_interface_vieworksvp;
                    my_interface_vieworksvp = 0;
                }

                if(my_camera_vieworksvp)
                {
                    delete my_camera_vieworksvp;
                    my_camera_vieworksvp = 0;
                }
            }
#endif
    
        }
    }
    catch(Tango::DevFailed& df)
    {
        //- rethrow exception
        throw LIMA_HW_EXC(Error, std::string(df.errors[0].desc).c_str());
    }
    catch(Exception& e)
    {
        throw LIMA_HW_EXC(Error, e.getErrMsg());
    }
    catch(...)
    {
        throw LIMA_HW_EXC(Error, "reset : Unknown Exception");
    }
}

//-----------------------------------------------------------------------------------------
//- force Init() on the specific device.
//-----------------------------------------------------------------------------------------
void ControlFactory::init_specific_device(const std::string& detector_type )
{
    yat::MutexLock scoped_lock(object_lock);
    try
    {
        //get the tango device/instance
        if(!ControlFactory::is_created)
        {
            std::string  detector = detector_type;
            Tango::DbDatum db_datum;
            my_server_name = Tango::Util::instance()->get_ds_name ();
            db_datum = (Tango::Util::instance()->get_database())->get_device_name(my_server_name,detector);
            db_datum >> my_device_name;
        }

        (Tango::Util::instance()->get_device_by_name(my_device_name))->delete_device();
        (Tango::Util::instance()->get_device_by_name(my_device_name))->init_device();
    }
    catch(Tango::DevFailed& df)
    {
        //- rethrow exception
        throw LIMA_HW_EXC(Error, std::string(df.errors[0].desc).c_str());
    }
}

//-----------------------------------------------------------------------------------------
//- call dev_state() command of the generic device.
//-----------------------------------------------------------------------------------------
Tango::DevState ControlFactory::get_state(void )
{
    yat::AutoMutex<> _lock(object_state_lock);
    return my_state;
}
//-----------------------------------------------------------------------------------------
//- call dev_status() command of the generic device.
//-----------------------------------------------------------------------------------------
std::string 	ControlFactory::get_status(void )
{
    yat::AutoMutex<> lock(object_state_lock);
    return (my_status.str());

}
//-----------------------------------------------------------------------------------------
void ControlFactory::set_state(Tango::DevState state) 
{
    yat::AutoMutex<> _lock(object_state_lock);
    my_state = state;
}

//-----------------------------------------------------------------------------------------
void ControlFactory::set_status (const std::string& status)
{      
    yat::AutoMutex<> _lock(object_state_lock);
    my_status.str("");
    my_status<<status.c_str()<<endl;

}
//-----------------------------------------------------------------------------------------


