const char DATA_index_html[] PROGMEM = R"=====(
<html>
<head>
<script src="/js/jquery.min.js"></script>
</head>
<body>
<script language="Javascript">
$(document).ready(function(){
   $('#out').html("<b>OK</b>");
});
</script>

Hallo <b>Welt</b>
<div id="out">

</div>
</body>
</html>
)=====";
