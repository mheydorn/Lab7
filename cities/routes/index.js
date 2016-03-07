var express = require('express');
var router = express.Router();
 var fs = require('fs');
/* GET home page. */
router.get('/', function(req, res, next) {
               res.sendFile('weather.html', { root:  'public' });

});

 router.get('/getcity',function(req,res,next) {
            console.log("In getcity route");
          var myRe = new RegExp("^" + req.query.q);
          console.log(myRe);
          var cities;
          fs.readFile(__dirname + '/cities.dat.txt',function(err,data) {
          if(err){ throw err; console.log("Could not open file!");}
          console.log("Here1");
	  cities = data.toString().split("\n");
          for(var i = 0; i < cities.length; i++) {
              //console.log(cities[i]);
//	  console.log("1");	     
          var result = cities[i].search(myRe);
          if(result != -1) {
            //console.log(cities[i]);
          }
            }

        var jsonresult = [];
        for(var i = 0; i < cities.length; i++) {
          var result = cities[i].search(myRe); 
          if(result != -1) {
           console.log(cities[i]);
           jsonresult.push({city:cities[i]});
          } 
        }
	res.status(200).json(jsonresult);   
        //console.log(jsonresult);
          })
  //       console.log("2");
        
	//res.status(200).json(jsonresult);
          
});

 router.get('/spell',function(req,res,next) {
            console.log("In spell route");
          var myRe = new RegExp("^" + req.query.q);
          console.log(myRe);
          var words;
          fs.readFile(__dirname + '/words.txt',function(err,data) {
          if(err){ throw err; console.log("Could not open file!");}
          words = data.toString().split("\n");
          
        var jsonresult = [];
	//if(words.length > 0){
	//	jsonresult.push({word:words.length.toString()});
	//	jsonresult.push({word:"another"});
	//}else{
	//	jsonresult.push({word:"notgood"});
	//}
	var howmany = 0;
	for(var i = 0; i < words.length; i++) { 
	if(req.query.q == words[i]) {
          //jsonresult.push({word:words[i]});
	 howmany++;
          }
        }
	jsonresult.push({cuantos:howmany});
        res.status(200).json(jsonresult);
          })
});


module.exports = router;
