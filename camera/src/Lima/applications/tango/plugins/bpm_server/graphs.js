var graphs_view = (function() {
  var current_id;
  var ui_box;
  var profile_x_canvas;
  var profile_y_canvas;
  var profile_x_plot;
  var profile_y_plot;
  var x_profile_data = { data: [] };
  var y_profile_data = { data: [] };
  var options = { grid: { backgroundColor: "#FFFFFF" },
                  xaxis: { min: 0, max: null },
                  yaxis: { min: 0, max: null, title: 'intensity' },
                  spreadsheet: { show: true },
                  parseFloat: false,
                  HtmlText: false,
                  mouse: { track: true },
                  selection: { mode: "x", fps: 30 } }

  var update_graph = function() {
     var max_y = null;
     var auto = Boolean(uki("#graphs_autoscale_"+current_id).value())
     if (! auto) {
       uki("#graphs_max_y_"+current_id).disabled(0);
       if (uki("#graphs_max_y_"+current_id).value() != "") {
         max_y = parseInt(uki("#graphs_max_y_"+current_id).value())
       }
     } else {
       uki("#graphs_max_y_"+current_id).disabled(1);
     }
     if (max_y == null) auto=true;

    var opts = { yaxis: { min: 0, max: max_y, title: 'intensity', autoscale: auto }};

    try {
        o = Flotr._.extend(Flotr._.clone(options), opts || {});

        o["title"] = "Projection along X axis"
        profile_x_plot = Flotr.draw(profile_x_canvas, x_profile_data, o);
  
        o["title"] = "Projection along Y axis"
        profile_y_plot = Flotr.draw(profile_y_canvas, y_profile_data, o);
    } catch (e) {
    }
  }

  return {
    init: function(parent, id) {
      current_id = id
    
      ui_box = uki({
        view: 'Box', rect: '0 0 660 620', anchors: 'left top', 
        childViews:[ 
                     { view: 'Checkbox', rect: '10 155 20 24', id: 'graphs_autoscale_'+current_id, anchors: 'left top' },
                     { view: 'Label', rect: '35 155 160 24', text: 'Autoscale Y axis ; max value = ', anchors: 'left top' },
                     { view: 'TextField', rect: '200 155 120 24', id: 'graphs_max_y_'+current_id, anchors: 'left top' },
                     { view: 'GraphCanvas', id: 'profile_x_'+current_id, rect: '10 190 640 250', anchors: 'left top' },
                     { view: 'GraphCanvas', id: 'profile_y_'+current_id, rect: '10 190 640 250', anchors: 'left top' } 
      ]})[0]
    
      parent.appendChild(ui_box)
      parent.layout()

      uki("#graphs_autoscale_"+current_id).checked(true);
      uki("#graphs_autoscale_"+current_id).bind('click', update_graph);
      uki("#graphs_max_y_"+current_id).change(update_graph);

      profile_x_canvas = uki("#profile_x_"+current_id).dom();
      profile_y_canvas = uki("#profile_y_"+current_id).dom();
  
      Flotr.EventAdapter.observe(profile_x_canvas, "flotr:select", function(area) {
        update_graph({ xaxis: { min: area.x1, max: area.x2 },
                       yaxis: { min: area.y1, max: area.y2 } })
      });
      Flotr.EventAdapter.observe(profile_x_canvas, "flotr:click", function() {
        update_graph()
      }); 

      Flotr.EventAdapter.observe(profile_y_canvas, "flotr:select", function(area) {
        update_graph({ xaxis: { min: area.x1, max: area.x2 },
                       yaxis: { min: area.y1, max: area.y2 } })
      });
      Flotr.EventAdapter.observe(profile_y_canvas, "flotr:click", function() {
        update_graph()
      });
 
      return ui_box
    },

    set_profile_xy: function(profile_x, profile_y) {
      x_profile_data.data = profile_x
      y_profile_data.data = profile_y
      update_graph()
    }
  }
});

