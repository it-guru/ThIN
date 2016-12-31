define(["action/SystemChangePasswd"], function(SystemChangePasswd) {  
return {    
   start: function() {      
      $("#main").html("SystemChangePasswd loadedX");      
         $("#main").html(
            FRM.form('passwd',
               FRM.fset('fset1','System - change password',
               "<div id='status'></div>"+
               FRM.fset('fset2','current password',
               FRM.fld('password','oldpassword','Password:',''))+   
               FRM.fset('fset3','new password',
               FRM.fld('password','newpassword','new:','')+   
                  FRM.fld('password','newpassword2','repeat:',''))+  
               FRM.fblk(FRM.btn('doChange','change password','15rem')))
            )
         );
         $('#doChange').click(function(){
            var str=$("#passwd").serialize();
            SpinnerOn();
            $.ajax({
              type: "POST",
              url: '/jssys/changePassword',
              data: str,
              success: function(data,textStatus,jqXHR){
                 if (data.exitcode>0){
                    alert("error "+data.exitmsg);
                 }
                 else{
                    $('#fset1','div').html("<font color=green>"+
                                     "OK password changed</font>");
                 }
                 SpinnerOff();
              },
              error: function(xreq,textStatus,errorThrown){
                 console.log("logon ERROR textStatus:",
                             textStatus,"error:",errorThrown," xreq",xreq);
                 SpinnerOff();
              },
              dataType: "jsonp"
            });
         });
      return true;    
   },    
   end: function() {      
      return true;    
   }  
}});
