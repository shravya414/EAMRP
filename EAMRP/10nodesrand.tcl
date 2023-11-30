puts "Enter number of nodes"
set nnodes [gets stdin]
set val(chan)   Channel/WirelessChannel    ;# channel type
set val(prop)   Propagation/TwoRayGround   ;# radio-propagation model
set val(netif)  Phy/WirelessPhy            ;# network interface type
set val(mac)    Mac/802_11                 ;# MAC type
set val(ifq)    Queue/DropTail/PriQueue    ;# interface queue type
set val(ll)     LL                         ;# link layer type
set val(ant)    Antenna/OmniAntenna        ;# antenna model
set val(ifqlen) 50                         ;# max packet in ifq
set val(nn)     $nnodes                         ;# number of mobilenodes
set val(rp)     EAMRP                       ;# routing protocol
set val(x)      800                      ;# X dimension of topography
set val(y)      800                      ;# Y dimension of topography
set val(stop)   20.0                         ;# time of simulation end

set ns [new Simulator]

set tracefd [open eamrp10.tr w]


set namtracefd [open eamrp10.nam w]

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
			 -rxPower 0.0395 \
			 -txPower 0.066 \
			 -sleepPower 0.000015 \
	 		 -initialEnergy 10 \
                -movementTrace ON 

for {set i 0} {$i < $val(nn) } {incr i} {
  set node($i) [$ns node]
 }

for {set i 0} {$i < $val(nn) } {incr i} {
$node($i) set X_ [expr rand()*$val(x)]
$node($i) set Y_ [expr rand()*$val(y)]
}


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


for {set i 0} {$i < 10} {incr i} {

  $ns at 0.5 "$node($i) setdest  10 10 5"
   
  
}





for {set i 0} {$i < $val(nn) } {incr i} {
 $ns initial_node_pos $node($i) 30
 }

puts "Enter source node"
set sn [gets stdin]

puts "Enter destination"
set dn [gets stdin]

$ns at 0.0 "$node($sn) label Source"
$ns at 0.0 "$node($dn) label Destination"


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


	$ns attach-agent $node($dn) $nulla
	set cbr0 [attach-cbr-traffic $node($sn) $nulla 64 0.01]
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
exec nam eamrp10.nam &
exit 0
}


$ns run
