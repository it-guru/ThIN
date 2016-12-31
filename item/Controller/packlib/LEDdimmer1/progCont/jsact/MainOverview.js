define(["action/MainOverview"], function(MainOverview) {
  return {
    start: function() {
        var curval;
        $("#main").html(
           FRM.form('dev',
           FRM.fset('fset1','LED Controller',
           "LED Level<br>"+
              '<input id=led type="range" min="0"  max="1000" step="1" > '
           ))
        );
        setInterval(function(){
           var newval=$('#led').val();
           if (newval!=curval){
              var value=newval-1;
              var l=value/1000.0;
              if (l<0){
                 l=0;
              }
              jQuery.ajax({
                 url:"../js/REST/set/LED/"+l,
                 contentType: "application/json; charset=utf-8",
                 dataType: "json",
                 success:function(data,st,jqXHR) {   
                    console.log("ajax ok:",data,st,jqXHR);
                 },
                 error:function(jqXHR,st) {   
                    console.log("ajax NOTOK:",jqXHR,st);
                 }
              });
              curval=newval;
           }
        },1000); 
        App.postSystemEvent("query:ALL");
        return true;
        },
    handleSystemEvent: function(evt) {
       var myRegexp = /state:C:(.*):(.*)/;
       var l=myRegexp.exec(evt.data);
       if (l[2]=="1"){
          $("#"+l[1]).prop('checked', true);
       }if (l[2]=="0"){
          $("#"+l[1]).prop('checked', false);
       }
       return true;
    },
    end: function() {
       return true;
    }
  }
});

