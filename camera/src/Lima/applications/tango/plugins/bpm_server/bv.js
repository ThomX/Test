var bv_view = (function() {
  var current_id;
  var ui_box;
  var img_canvas;
  var img_context;
  var img_width;
  var img_height;
  var img_socket = null;
  var img_num = 0;
  var live_mode = false;
  var jpeg = new Image();
  var graphs = new graphs_view();
  var extchanges_socket = null;
  var calib_x = 1;
  var calib_y = 1;
  var roi = [0,0,0,0];
  var fwhm_x;
  var fwhm_y;
  var proj_x = [];
  var proj_y = [];
  var bi;
  var bx;
  var by;
  var autoscale_options = [{ value: 0, text: "Linear"}, {value: 1, text:"Logarithmic"}]; 
  var graphs_display_timer = null;
  var beam_mark_pos = [0,0];
  var background = false;
  var last_jpeg_data = null;

  var initCanvas = function(width, height) {
    img_canvas.width = width
    img_canvas.height = height
    img_context.mozImageSmoothingEnabled = false
    img_context.fillStyle="#808080"
    img_context.fillRect(0,0, width, height)
    img_context.strokeStyle = "#000000"

    var uki_img_canvas = uki("#canvas_"+current_id)
    var img_canvas_rect = uki_img_canvas.rect()
    img_canvas_rect.width = width
    img_canvas_rect.height = height
    uki_img_canvas.rect(img_canvas_rect)

    var cursor_pos_rect = uki("#cursor_pos_"+current_id).rect()
    cursor_pos_rect.x = img_canvas_rect.x + img_canvas_rect.width - cursor_pos_rect.width
    uki("#cursor_pos_"+current_id).rect(cursor_pos_rect)

    var cmd_roi_rect = uki("#cmd_set_roi_"+current_id).rect()
    cmd_roi_rect.y = img_canvas_rect.y + img_canvas_rect.height
    uki("#cmd_set_roi_"+current_id).rect(cmd_roi_rect)
    var lbl_roi_rect = uki("#roi_coordinates_"+current_id).rect()
    lbl_roi_rect.y = img_canvas_rect.y + img_canvas_rect.height
    uki("#roi_coordinates_"+current_id).rect(lbl_roi_rect)

    uki("#image_view_"+current_id).resizeToContents("width height").parent().resizeToContents("width height").layout();
  }

  var initCtrl = function(exposure_time, acq_rate, color_map, autoscale, do_bpm, roi, live, cx, cy, background, bx, by) {
    uki("#exposure_time_"+current_id).value(exposure_time)
    if (roi) uki("#cmd_set_roi_"+current_id).text("Reset ROI");
    uki("#acquisition_rate_"+current_id).value(acq_rate)
    uki("#color_map_"+current_id).checked(color_map)
    uki("#autoscale_"+current_id).checked(autoscale)
    uki("#bpm_enabled_"+current_id).checked(do_bpm) 

    live_mode = live 
    background = background

    uki("#txt_calib_x_"+current_id).value(cx);
    uki("#txt_calib_y_"+current_id).value(cy);
    update_calibration();

    beam_mark_pos = [bx, by];
    if (bx > 0) {
        uki("#cmd_lock_beam_mark_"+current_id).checked(true);
    } else {
        uki("#cmd_lock_beam_mark_"+current_id).checked(false);
    }
  
    get_external_changes();
    get_images();
  }

  var receive_external_change = function(res) {
      uki('#exposure_time_'+current_id).value(res.exp_t)

      if (res.background) {
        background = true;
      } else {
        background = false;
      }

      if (res.in_acquisition) {
        live_mode = true;
      } else {
        live_mode = false;
      }

      update_control();
  }

  var receive_img = function(res) {
      img_num = img_num + 1
      uki('#frame_number_'+current_id).text("Frame "+img_num)

      proj_x = res.profile_x;
      proj_y = res.profile_y;
      fwhm_x = res.fwhm_x;
      fwhm_y = res.fwhm_y;
      roi = res.roi;
      bi = res.I;
      bx = res.X;
      by = res.Y;
      last_jpeg_data = res.jpegData;

      _update_calibration(); 

      display_img(res.jpegData);
  }

  var draw_beam_mark = function() {
                if ((beam_mark_pos[0] > 0)&&(beam_mark_pos[1] > 0)) {
                  if (uki("#color_map_"+current_id).value()) {
                    img_context.strokeStyle = "#FFFFFF" //white
                  } else {
                    img_context.strokeStyle = "#FF0000" //red 
                  }

                  img_context.beginPath();
                  img_context.moveTo(0, beam_mark_pos[1]);
                  img_context.lineTo(img_width, beam_mark_pos[1]);
                  img_context.stroke();
                  img_context.moveTo(beam_mark_pos[0], 0);
                  img_context.lineTo(beam_mark_pos[0], img_height);
                  img_context.stroke();
                  img_context.closePath();
                }
  }

  var display_img = function(jpeg_data) {
      jpeg.onload = function() {
                img_width = img_canvas.width;
                img_height = img_canvas.height; 
                var aspect_ratio = jpeg.width / jpeg.height; //img_width / img_height;

                if ((img_width != jpeg.width)||(img_height != jpeg.height)) {
                  img_context.fillStyle="#808080"
                  img_context.fillRect(0,0, img_canvas.width, img_canvas.height);

                  if (aspect_ratio > 1) {
                    // width is bigger than height
                    img_height /= aspect_ratio;
                    img_context.drawImage(jpeg, 0, 0, img_width, img_height)
                  } else {
                    img_width *= aspect_ratio;
                    img_context.drawImage(jpeg, 0, 0, img_width, img_height)
                  }
                } else {
                  img_context.drawImage(jpeg, 0, 0, img_width, img_height)
                }

                draw_beam_mark();

                if (uki("#color_map_"+current_id).value()) {
                  img_context.strokeStyle = "#000000" //black
                } else {
                  img_context.strokeStyle = "#7FFF00" //chartreuse
                }

                var x = bx * (img_width / jpeg.width) / 2
                var y = by * (img_height / jpeg.height) / 2
                var fwhm_x = fwhm_x * (img_width / jpeg.width) / 2
                var fwhm_y = fwhm_y * (img_height / jpeg.height) / 2

                img_context.beginPath();
                img_context.moveTo(0, y);
                img_context.lineTo(img_width, y);
                img_context.stroke();
                img_context.moveTo(x, 0);
                img_context.lineTo(x, img_height);
                img_context.stroke();
                img_context.closePath();
                img_context.strokeRect(x-(fwhm_x/2), y-(fwhm_y/2), fwhm_x, fwhm_y);
      }
      jpeg.src = "data:image/png;base64,"+jpeg_data;
  }

  var get_external_changes = function() {
    var ws_address;
    if (window.location.port != "") {
        ws_address = "ws://"+window.location.hostname+":"+window.location.port+"/ext_changes_channel";
    } else {
        ws_address = "ws://"+window.location.hostname+"/ext_changes_channel";
    }
    try { 
      extchanges_socket = new WebSocket(ws_address);
    } catch(e) {
      extchanges_socket = new MozWebSocket(ws_address);
    }
    extchanges_socket.onopen = function() {
      extchanges_socket.send(current_id.toString());
    };
    extchanges_socket.onmessage = function(packed_msg) {
      receive_external_change(JSON.parse(packed_msg.data));  
      extchanges_socket.send(current_id.toString());
    };
  }

  var get_images = function() {
    if ((img_num == 0)&&(img_socket==null)) {
      var ws_address;
      if (window.location.port != "") {
        ws_address = "ws://"+window.location.hostname+":"+window.location.port+"/image_channel";
      } else {
        ws_address = "ws://"+window.location.hostname+"/image_channel";
      }
      try {
        img_socket = new WebSocket(ws_address);
      } catch(e) {
        img_socket = new MozWebSocket(ws_address);
      }
      img_socket.onopen = function() {
        img_socket.send(current_id.toString()); 
      };
      img_socket.onmessage = function(packed_msg) {
        receive_img(JSON.parse(packed_msg.data)); 
        get_images();
      };
    } else { 
      var visible = uki("#panel_"+current_id).visible();
      if (visible) {
        img_socket.send(current_id.toString());
      } else {
        setTimeout(get_images, 100);
      }
    }
  }

  var set_img_display_config = function() {
    var as;
    var cm;
    if (uki("#color_map_"+current_id).value()) { cm = 1 } else { cm = 0 };
    if (uki("#autoscale_"+current_id).value()) { as = 1 } else { as = 0 };
   
    var autoscale_opt = uki("#autoscale_option_"+current_id).value();
    autoscale_opt_value = autoscale_options[autoscale_opt].text;
 
    $.ajax({
          error: function(XMLHttpRequest, textStatus, errorThrown) {},
          url: 'img_display_config',
          type: 'GET',
          async: false,
          success: function(res) { },
          data: { query:"set_img_display_config", client_id: current_id, color_map: cm, autoscale: as, autoscale_option: autoscale_opt_value },
          dataType: 'json'
    });
  }


  var take_background = function() {
    $.ajax({
          error: function(XMLHttpRequest, textStatus, errorThrown) {},
          url: 'set_background',
          type: 'GET',
          success: function(res) {}, 
          data: { query:"set_background", client_id: current_id, set:1 },
          dataType: 'json'
    });
  }


  var reset_background = function() {
    $.ajax({
          error: function(XMLHttpRequest, textStatus, errorThrown) {},
          url: 'set_background',
          type: 'GET',
          success: function(res) {},
          data: { query:"set_background", client_id: current_id, set:0 },
          dataType: 'json'
    });
  }


  var get_beam_pos = function() {
    // JSON converts boolean values to strings!
    var live;
    var do_bpm;
    var exposure_time = uki("#exposure_time_"+current_id).value();
    if (live_mode) { live = 1 } else { live = 0 };
    if (uki("#bpm_enabled_"+current_id).value()) { do_bpm = 1 } else { do_bpm = 0 };

    set_img_display_config()
    update_calibration()

    $.ajax({
          error: function(XMLHttpRequest, textStatus, errorThrown) {},
          url: 'get_beam_position',
          type: 'GET',
          async: false,
          success: function(res) { },
          data: { query:"get_beam_position", client_id: current_id, live: live, exp_t: exposure_time, do_bpm: do_bpm, acq_rate: uki("#acquisition_rate_"+current_id).value() },
          dataType: 'json'
    });
 
    setTimeout(update_graphs, exposure_time*1300); 
  }

  var update_control = function() {
      uki("#exposure_time_"+current_id).disabled(1)
      uki("#exposure_time_"+current_id)[0]._input.setAttribute('readonly', 'readonly')
      uki("#acquisition_rate_"+current_id).disabled(1)
      uki("#acquisition_rate_"+current_id)[0]._input.setAttribute('readonly', 'readonly')
      uki('#cmd_get_beam_pos_'+current_id).disabled(1)
      uki('#cmd_get_beam_pos_'+current_id).unbind("click", get_beam_pos)
      uki("#cmd_set_roi_"+current_id).disabled(1)
      uki("#cmd_set_roi_"+current_id).unbind("click", set_roi)
      uki("#bpm_enabled_"+current_id)[0].disabled(1)
      uki("#cmd_reset_background_"+current_id).disabled(1)
      uki("#cmd_reset_background_"+current_id).unbind('click', reset_background)
      uki("#cmd_take_background_"+current_id).disabled(1)
      uki("#cmd_take_background_"+current_id).unbind('click', take_background)

      if (live_mode) {
        uki("#cmd_live_"+current_id).checked(1)
      } else {
        uki("#exposure_time_"+current_id).disabled(0)
        uki("#exposure_time_"+current_id)[0]._input.removeAttribute('readonly')
        uki("#acquisition_rate_"+current_id).disabled(0)
        uki("#acquisition_rate_"+current_id)[0]._input.removeAttribute('readOnly')
        uki('#cmd_get_beam_pos_'+current_id).disabled(0) 
        uki('#cmd_get_beam_pos_'+current_id).bind("click", get_beam_pos)
        uki("#cmd_set_roi_"+current_id).disabled(0)
        uki("#cmd_set_roi_"+current_id).bind("click", set_roi)
        uki("#bpm_enabled_"+current_id)[0].disabled(0)
        uki("#cmd_live_"+current_id).checked(0) 
        if (background) {
          uki("#cmd_reset_background_"+current_id).bind('click',reset_background)
          uki("#cmd_reset_background_"+current_id).disabled(0)
        } else {
          uki("#cmd_take_background_"+current_id).bind('click', take_background)
          uki("#cmd_take_background_"+current_id).disabled(0)
        }
      }
  }

  var start_stop_live = function() {
    if (live_mode) {
      live_mode = false;
    } else {
      live_mode = true;
    }
    update_control();
    get_beam_pos();
  }

  var update_graphs = function() {
    var proj_x_calibrated;
    var proj_y_calibrated;
 
    // there is no deep copy in javascript...
    // here, slice(0) cannot work because the array contains sub-arrays
    // the JSON trick is ugly but it works!
    proj_x_calibrated = JSON.parse(JSON.stringify(proj_x));
    proj_y_calibrated = JSON.parse(JSON.stringify(proj_y));
    for (var i=0; i<proj_x.length; i++) {
      proj_x_calibrated[i][0]*= calib_x;
    }
    for (var i=0; i<proj_y.length;i++) {
      proj_y_calibrated[i][0]*= calib_y;
    }

    graphs.set_profile_xy(proj_x_calibrated, proj_y_calibrated);

    if (live_mode) setTimeout(update_graphs, 300);
  }

  var _update_calibration = function() {
    var digix; var digiy;

    calib_x = uki("#txt_calib_x_"+current_id).value();
    calib_y = uki("#txt_calib_y_"+current_id).value();
    digix = 2; digiy = 2;
    //if (Math.abs(calib_x - 1)<1E-4) { digix = 0 } else { digix = 2 };
    //if (Math.abs(calib_y - 1)<1E-4) { digiy = 0 } else { digiy = 2 };

    uki('#fwhm_'+current_id).text("Fwhm "+(calib_x*fwhm_x).toFixed(digix)+" x "+(calib_y*fwhm_y).toFixed(digiy))

    uki("#roi_coordinates_"+current_id).text("X="+(calib_x*roi[0]).toFixed(digix)+",Y="+(calib_y*roi[1]).toFixed(digiy)+",W="+(calib_x*roi[2]).toFixed(digix)+",H="+(calib_y*roi[3]).toFixed(digiy));
  
    uki('#intensity_'+current_id).text("Beam: intensity="+bi+" bx="+(bx*calib_x).toFixed(digix)+" by="+(by*calib_y).toFixed(digiy))
  }

  var save_calibration = function() { 
     _update_calibration() 
     update_graphs()

    $.ajax({
         error: function(XMLHttpRequest, textStatus, errorThrown) {},
         url: 'update_calibration',
         type: 'GET',
         async: false,
         success: function(res) { },
         data: { query:"update_calibration", client_id: current_id, x: calib_x, y: calib_y, save:1 },
         dataType: 'json' });
  }

  var update_calibration = function() { 
     _update_calibration() 
     update_graphs()

    $.ajax({
         error: function(XMLHttpRequest, textStatus, errorThrown) {},
         url: 'update_calibration',
         type: 'GET',
         success: function(res) { },
         data: { query:"update_calibration", client_id: current_id, x: calib_x, y: calib_y, save:0 },
         dataType: 'json' });
  }

  var update_mark_pos_intensity = function(intensity) {
     draw_beam_mark();
     img_context.textBaseline = "bottom"
     img_context.fillStyle = img_context.strokeStyle
     img_context.font = '12px sans-serif bold';
     img_context.fillText("I="+intensity, beam_mark_pos[0]+3, beam_mark_pos[1]-3)
  }

  var get_mark_pos_intensity = function() {
     var bx;
     var by;
     if (roi_w == 0) {
       bx = beam_mark_pos[0];
       by = beam_mark_pos[1];
     } else {
       bx = Math.round(0.5*beam_mark_pos[0]*roi_w/img_width);
       by = Math.round(0.5*beam_mark_pos[1]*roi_h/img_height);
     }
      
     $.ajax({
         error: function(XMLHttpRequest, textStatus, errorThrown) {},
         url: 'get_intensity',
         type: 'GET',
         async: false,
         success: function(res) { update_mark_pos_intensity(res.intensity) },
         data: { query:"get_intensity", client_id: current_id, x: bx, y: by },
         dataType: 'json' 
     });
  }

  var lock_beam_mark = function() {
    if (uki("#cmd_lock_beam_mark_"+current_id).value()) {
      $.ajax({
          error: function(XMLHttpRequest, textStatus, errorThrown) { uki("#cmd_lock_beam_mark_"+current_id).checked(false); },
          url: 'lock_beam_mark',
          type: 'GET',
          success: function(res) { uki("#cmd_lock_beam_mark_"+current_id).checked(true); },
          data: { query:"lock_beam_mark", client_id: current_id, x: beam_mark_pos[0], y: beam_mark_pos[1] },
          dataType: 'json' });
    } else {
      $.ajax({
          error: function(XMLHttpRequest, textStatus, errorThrown) { },
          url: 'lock_beam_mark',
          type: 'GET',
          success: function(res) { },
          data: { query:"lock_beam_mark", client_id: current_id, x: 0, y: 0 },
          dataType: 'json' });
    }
  }

  var clear_beam_mark = function() {
    beam_mark_pos = [0,0];
    uki("#cmd_lock_beam_mark_"+current_id).checked(false);
    lock_beam_mark();
    display_img(last_jpeg_data);
  }

  var set_roi = function() {
    beam_mark_pos = [0,0];

    if (uki("#cmd_set_roi_"+current_id).text() == "Set ROI") {
      $.ajax({
          error: function(XMLHttpRequest, textStatus, errorThrown) {},
          url: 'set_roi',
          type: 'GET',
          success: function(res) { uki("#cmd_set_roi_"+current_id).text("Reset ROI"); get_beam_pos() },
          data: { query:"set_roi", client_id: current_id, x: roi_x, y: roi_y, w: roi_w, h: roi_h }, 
          dataType: 'json' });
    } else {
      $.ajax({
          error: function(XMLHttpRequest, textStatus, errorThrown) {},
          url: 'set_roi',
          type: 'GET',
          success: function(res) { uki("#cmd_set_roi_"+current_id).text("Set ROI"); get_beam_pos() },
          data: { query:"set_roi", client_id: current_id, x: 0, y:0, w:0, h:0 }, 
          dataType: 'json' });
    }          
  }

  var roi_x = 0;
  var roi_y = 0;
  var roi_w = 0;
  var roi_h = 0;

  rect_tool = function () {
     var tool = this;
     this.started = false;
     this.single_point = false;

     this.mousedown = function (ev) {
       if ((!live_mode)&&(img_num>0)) {
         tool.x0 = ev._x;
         tool.y0 = ev._y;
         tool.started=true;
       }
     };

     this.mousemove = function (ev) {
      if (!tool.started) {
         return;
      }
      
      var x = Math.min(ev._x,  tool.x0),
          y = Math.min(ev._y,  tool.y0),
          w = Math.abs(ev._x - tool.x0),
          h = Math.abs(ev._y - tool.y0);

      img_context.drawImage(jpeg, 0, 0, img_width, img_height);

      if (!w || !h) {
        tool.single_point = true;
        return;
      } else {
        tool.single_point = false;
      }

      img_context.strokeRect(x, y, w, h);
      roi_x = x*2;
      roi_y = y*2;
      roi_w = w*2;
      roi_h = h*2;
    };

    this.mouseup = function (ev) {
      tool.mousemove(ev);
      tool.started=false;

      if (tool.single_point) {
        if ((ev._x > img_width) || (ev._y > img_height)) return;
        if (! uki("#cmd_lock_beam_mark_"+current_id).value()) {
          beam_mark_pos = [ev._x, ev._y];
        }
        //draw_beam_mark();
        get_mark_pos_intensity();
      }
    };
  };

  var tool = new rect_tool();

  function ev_canvas (ev) {
     ev._x = ev.layerX;
     ev._y = ev.layerY;
   
     cursor_x = 2*ev._x
     cursor_y = 2*ev._y
     if (jpeg.width > 0) {
       cursor_x = cursor_x / (img_width / jpeg.width)
       cursor_y = cursor_y / (img_height / jpeg.height)
     }
     uki("#cursor_pos_"+current_id).text("X="+Math.round(calib_x*cursor_x)+" ; Y="+Math.round(calib_y*cursor_y))

     tool[ev.type](ev);
  }

  return { init: function(parent, id) {
    current_id = id
    
    ui_box = uki({
      view: 'Box', id:'box_'+current_id, rect: '0 0 1230 1000', anchors: 'left top', 
      childViews: [
                   { view: 'Label', rect: '10 10 90 24', anchors: 'left top', text: 'Exposure time (s): ' }, 
                   { view: 'TextField', rect: '130 10 100 24', anchors: 'left top', id: 'exposure_time_'+current_id },
                   { view: 'Button', id: 'cmd_get_beam_pos_'+current_id, rect: '235 10 150 24', anchors: 'left top', text: 'One shot'},
                   { view: 'Label', rect: '10 44 90 24', anchors: 'left top', text: 'Sampling rate (Hz): ' },
                   { view: 'TextField', rect: '130 44 100 24', anchors: 'left top', id: 'acquisition_rate_'+current_id }, 
                   { view: "uki.more.view.ToggleButton", id: "cmd_live_"+current_id, text: "Live", anchors: 'left top', rect: '235 44 150 50' },
                   { view: "Button", id: 'cmd_take_background_'+current_id, rect: "395 10 150 24", anchors: 'left top', text: 'Take background' },
                   { view: "Button", id: "cmd_reset_background_"+current_id, rect: "395 44 150 24", anchors: "left top", text: "Reset background" },
                   { view: 'Checkbox', rect: '10 95 20 24', anchors:'left top', id:'color_map_'+current_id, checked: true },
                   { view: 'Label', rect: '40 95 90 24', anchors: 'left top', text: 'Temperature color map' },
                   { view: 'Checkbox', rect: '395 75 20 24', anchors: 'left top', id: 'bpm_enabled_'+current_id, checked: true },
                   { view: 'Label', rect: '425 75 100 23', anchors: 'left top', text: "Enable algorithm" },
                   { view: 'Label', rect: '10 120 20 23', anchors: 'left top', text: "LUT" },
                   { view: 'uki.more.view.Select', rect: '40 120 120 24', anchors: 'left top', options: autoscale_options, rowHeight: 22, id: "autoscale_option_"+current_id },
                   { view: 'Checkbox', rect: '175 120 20 23', anchors: 'left top', id: 'autoscale_'+current_id, checked: false },
                   { view: 'Label', rect: '200 120 90 24', anchors: 'left top', text: 'Autoscale' },
                   { view: 'Box', rect: '10 150 500 30', anchors: 'left top', childViews: [
                       { view: "Label", text: 'Pixel size (X):', rect: '0 0 70 24', anchors: 'left top' },
                       { view: "TextField", id: "txt_calib_x_"+current_id, rect: '75 0 55 24', anchors: 'left top' },
                       { view: "Label", text: '(Y):', rect: '135 0 20 24', anchors: 'left top'},
                       { view: "TextField", id: "txt_calib_y_"+current_id, rect: '160 0 55 24', anchors:'left top'},
                       { view: 'Button', id: 'cmd_update_cal_'+current_id, rect: '225 0 70 24', anchors: 'left top', text: 'Update' },
                       { view: 'Button', id: 'cmd_save_cal_'+current_id, rect: '305 0 70 24', anchors: 'left top', text:'Save' },
                   ]},
                   { view: 'Box', rect: '10 185 700 638', id: 'image_view_'+current_id, anchors: 'left top', childViews: [
                       { view: "Label", id:"frame_number_"+current_id, text: "Frame ?", anchors: 'left top', rect: '0 0 50 24' },
                       { view: "Label", id:"intensity_"+current_id, text: "Beam: intensity=? bx=? by=?", anchors: "left top", rect: "100 0 150 24" },
                       { view: "Label", id:"fwhm_"+current_id, text: "Fwhm ?", anchors: "left top", rect: "340 0 50 24" },
		       { view: "Label", id:"cursor_pos_"+current_id, text:"X=?, Y=?", anchors:"left top", rect: '315 0 75 24' },
                       { view: "Button", id: 'cmd_set_roi_'+current_id, text: "Set ROI", anchors: 'left top', rect: '0 514 150 24' },
                       { view: "Label", id: "roi_coordinates_"+current_id, text: "X=0,Y=0,W=0,H=0", anchors: "left top", rect: "160 514 150 24" },
                       { view: "Label", text:"Click on image to set crosshair", anchors: "left top", rect:"350 507 200 24" },
                       { view: "uki.more.view.ToggleButton", id: "cmd_lock_beam_mark_"+current_id, "text": "Lock", anchors: "left top", rect:"550 507 50 24" },
                       { view: "Button", id: "cmd_clear_beam_mark_"+current_id, "text": "Clear", anchors: "left top", rect:"600 507 50 24" },
                       { view: "Canvas", id: 'canvas_'+current_id, rect: "0 24 700 580", anchors: "left top" } ],
                   },
                   { view: 'Box', rect: '700 0 1000 1000', id: 'graphs_box_'+current_id, anchors: 'left top' },
    ]})[0]
    
    parent.appendChild(ui_box)

    parent.layout()

    img_canvas = document.getElementById('canvas_'+current_id)
    img_context = img_canvas.getContext("2d");
 
    img_canvas.addEventListener('mousedown', ev_canvas, false);
    img_canvas.addEventListener('mousemove', ev_canvas, false);
    img_canvas.addEventListener('mouseup',   ev_canvas, false);
  
    uki("#cmd_get_beam_pos_"+current_id).bind('click', get_beam_pos)
    uki("#cmd_live_"+current_id).bind('click', start_stop_live)
    uki("#cmd_set_roi_"+current_id).bind('click', set_roi)
    uki("#autoscale_"+current_id).bind('click', set_img_display_config)
    uki("#color_map_"+current_id).bind('click', set_img_display_config)
    uki("#cmd_update_cal_"+current_id).bind('click', update_calibration)
    uki("#cmd_save_cal_"+current_id).bind('click', save_calibration)
    uki("#autoscale_option_"+current_id).change(set_img_display_config)
    uki("#cmd_take_background_"+current_id).bind('click', take_background)
    uki("#cmd_reset_background_"+current_id).bind('click',reset_background)
    uki("#cmd_lock_beam_mark_"+current_id).bind('click',lock_beam_mark)
    uki("#cmd_clear_beam_mark_"+current_id).bind('click', clear_beam_mark)

    $.ajax({
      error: function(XMLHttpRequest, textStatus, errorThrown) {},
      url: 'get_status',
      type: 'GET',
      success: function(res) {
        initCanvas(res.full_width, res.full_height)
        initCtrl(res.exposure_time, res.acq_rate, res.color_map, res.autoscale, res.bpm_on, res.roi, res.live, res.calib_x, res.calib_y, res.background, res.beam_mark_x, res.beam_mark_y)
        graphs.init(uki("#graphs_box_"+current_id), current_id)
      },
      data: { "client_id": id },
      dataType: 'json',
      async: false
    })

    return ui_box
  }
  }
});

