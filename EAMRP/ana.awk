BEGIN {
   maxpac_id=0;
   recevepkt = 0;
   sendspkt = 0;
   routingpkts = 0;
   receivespkt = 0;
   sum = 0;

}
{
   event = $1;
   tempo = $3;
   pac_id = $47;
   agt = $19;
   ttcp = $35;

   

   if ( event == "s" && agt=="AGT" && ttcp =="cbr")
	sendspkt++;

   if ( (event == "s" || event == "f") && agt=="RTR" &&  $7 =="EAMRP")
	rountingpkts++;

   if ( event == "r" && agt=="AGT" && ttcp =="cbr" )
	receivespkt++;

  
   if ( pac_id > maxpac_id ) maxpac_id = pac_id;
   if ( ! ( pac_id in timpIn ) ) timpIn[pac_id] = tempo;
   if ( event != "d" ) {
        if ( event == "r" ) tempoFim[pac_id] = tempo;
   } else tempoFim[pac_id] = 0;
}
END {
  
  for ( pac_id = 0; pac_id <= maxpac_id + 1; pac_id++ ) {
      duracao =  tempoFim[pac_id] - timpIn[pac_id];
      if ( duracao > 0 ) {
         
         sum = sum + duracao;
     }
  }

  printf("Packets Sent = %d pkt\n", sendspkt);
  printf("Packets Received = %d pkt\n", receivespkt);
  printf("Routing Overhead = %d pkt\n", rountingpkts);
  printf("Packets loss = %d pkt\n", sendspkt-receivespkt);
  #printf("Routing Load = %f pkt\n", rountingpkts/receivespkt);
  printf("Packet Delivery Fraction = %f %\n", receivespkt/sendspkt*100);
  printf("Average End-to-end delay = %.6f s\n", sum / receivespkt);


exit 0
}

