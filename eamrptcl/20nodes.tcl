
set val(chan)   Channel/WirelessChannel    ;# channel type
set val(prop)   Propagation/TwoRayGround   ;# radio-propagation model
set val(netif)  Phy/WirelessPhy            ;# network interface type
set val(mac)    Mac/802_11                 ;# MAC type
set val(ifq)    Queue/DropTail/PriQueue    ;# interface queue type
set val(ll)     LL                         ;# link layer type
set val(ant)    Antenna/OmniAntenna        ;# antenna model
set val(ifqlen) 50                         ;# max packet in ifq
set val(nn)     20                         ;# number of mobilenodes
set val(rp)     EAMRP                       ;# routing protocol
set val(x)      800                      ;# X dimension of topography
set val(y)      800                      ;# Y dimension of topography
set val(stop)   150.0                         ;# time of simulation end

set ns [new Simulator]

set tracefd [open eamrp20.tr w]


set namtracefd [open eamrp20.nam w]

$ns trace-all $tracefd
$ns use-newtrace

$ns namtrace-all-wireless $namtracefd $val(x) $val(y)

set topo [new Topography]
$topo load_flatgrid $val(x) $val(y)

create-god $val(nn)
set chan [new $val(chan)];#Create wireless channel


#===================================
#     Mobile node parameter setup
#===================================
$ns node-config -adhocRouting  $val(rp) \
                -llType        $val(ll) \
                -macType       $val(mac) \
                -ifqType       $val(ifq) \
                -ifqLen        $val(ifqlen) \
                -antType       $val(ant) \
                -propType      $val(prop) \
                -phyType       $val(netif) \
                -channel       $chan \
                -topoInstance  $topo \
                -agentTrace    ON \
                -routerTrace   ON \
                -macTrace      ON \
		-energyModel EnergyModel \
		-idlePower 0.035 \
			 -rxPower 0.002 \
			 -txPower 0.003 \
			 -sleepPower 0.000015 \
	 		 -initialEnergy 10 \
                -movementTrace ON 

for {set i 0} {$i < $val(nn) } {incr i} {
  set node($i) [$ns node]
 }

$node(0) set X_ 203
$node(0) set Y_ 73
$node(1) set X_ 256
$node(1) set Y_ 237
$node(2) set X_ 362
$node(2) set Y_ 201
$node(3) set X_ 398
$node(3) set Y_ 107
$node(4) set X_ 325
$node(4) set Y_ 35
$node(5) set X_ 484
$node(5) set Y_ 52
$node(6) set X_ 551
$node(6) set Y_ 138
$node(7) set X_ 531
$node(7) set Y_ 221
$node(8) set X_ 466
$node(8) set Y_ 289
$node(9) set X_ 378
$node(9) set Y_ 348
$node(10) set X_ 249
$node(10) set Y_ 376
$node(11) set X_ 156
$node(11) set Y_ 275
$node(12) set X_ 175
$node(12) set Y_ 437
$node(13) set X_ 309
$node(13) set Y_ 419
$node(14) set X_ 408
$node(14) set Y_ 408
$node(15) set X_ 508
$node(15) set Y_ 370
$node(16) set X_ 644
$node(16) set Y_ 343
$node(17) set X_ 640
$node(17) set Y_ 247
$node(18) set X_ 627
$node(18) set Y_ 126
$node(19) set X_ 580
$node(19) set Y_ 53
# Neighbors Distence Calucation


   	proc Distance { } {
      			   global node ns val	
		
			   set nfp [open "Distanc.Cal" w ]  
			   set  $nfp "'''''''''''''''''''''''''''''''''''''''''''''''"	
			   puts $nfp "Node\t\tNeighbors\tx-cor\ty-cor\tDistance "
			   set  $nfp "'''''''''''''''''''''''''''''''''''''''''''''''''''"

                	   for { set i 0 } { $i < $val(nn) } { incr i }   { 
 							   	set p 0					
								set x1 [$node($i) set X_]
								set y1 [$node($i) set Y_]
			   for { set j 0 } { $j < $val(nn) } { incr j }  {
 								set x2 [$node($j) set X_]
								set y2 [$node($j) set Y_]		        
			  
			   set dis  [ expr int(sqrt(pow([expr $x2 - $x1],2)+pow([expr $y2 - $y1],2)))]
	  	  		  
			   if { $dis < 300	 && $i!=$j } { incr p 	
	     		   puts $nfp "$i\t\t$j\t\t [expr int($x1)]\t[expr int($y1)]\t$dis"  }  }
			   puts $nfp "Total_Neighbors $i = $p"
                        				      } 
                           close $nfp   
		}
$ns at 0.8 "Distance"

for {set i 0} {$i < $val(nn) } {incr i} {
 $ns initial_node_pos $node($i) 30
 }
for {set i 0} {$i < 5} {incr i} {

  $ns at 10.0 "$node($i) setdest 10 10 5"
   
  
}

for {set i 5} {$i < $val(nn)} {incr i} {

  $ns at 20.0 "$node($i) setdest  10 10 5"
   
  
}
for {set i 0} {$i < 5} {incr i} {

  $ns at 30.0 "$node($i) setdest  300 300 5"
   
  
}



proc attach-cbr-traffic { node sink size interval } {
	global ns
	set source [new Agent/UDP]
	#$source set class_ 2
	$ns attach-agent $node $source
	set traffic [new Application/Traffic/CBR]
	$traffic set packetSize_ $size
	$traffic set interval_ $interval
	$traffic attach-agent $source
	$traffic set rate_ 10kbps
	$ns connect $source $sink
	return $traffic
}
set nulla [new Agent/LossMonitor]

	$ns attach-agent $node(9) $nulla
	set cbr0 [attach-cbr-traffic $node(0) $nulla 512 0.5]
$ns at 1.0 "$cbr0 start"
$ns at $val(stop) "$cbr0 stop"




for {set i 0} {$i < $val(nn) } {incr i} {
 $ns at $val(stop) "$node($i) reset"
}

$ns at $val(stop) "stop"

proc stop {} {
global ns tracefd namtracefd
$ns flush-trace
close $tracefd
close $namtracefd
exec nam eamrp20.nam &
exit 0
}


$ns run
