set val(chan)   Channel/WirelessChannel    ;# channel type
set val(prop)   Propagation/TwoRayGround   ;# radio-propagation model
set val(netif)  Phy/WirelessPhy            ;# network interface type
set val(mac)    Mac/802_11                 ;# MAC type
set val(ifq)    Queue/DropTail/PriQueue    ;# interface queue type
set val(ll)     LL                         ;# link layer type
set val(ant)    Antenna/OmniAntenna        ;# antenna model
set val(ifqlen) 50                         ;# max packet in ifq
set val(nn)     50                        ;# number of mobilenodes
set val(rp)     EAMRP                       ;# routing protocol
set val(x)      1200                      ;# X dimension of topography
set val(y)      1200                      ;# Y dimension of topography
set val(stop)   10.0                         ;# time of simulation end

#===================================
#        Initialization        
#===================================
#Create a ns simulator
set ns [new Simulator]

#Setup topography object
set topo       [new Topography]
$topo load_flatgrid $val(x) $val(y)
create-god $val(nn)

#Open the NS trace file

set rt [open Routingtable.tr w]
$ns trace-all $rt

set tracefile [open out.tr w]
$ns trace-all $tracefile

#Open the NAM trace file
set namfile [open out.nam w]
$ns namtrace-all $namfile
$ns namtrace-all-wireless $namfile $val(x) $val(y)
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

#===================================
#        Nodes Definition        
#===================================

#Energy
for {set i 1} {$i<$val(nn)} {incr i} {
	set e($i) 10
	# $ns at 0.0 "$n($i) label $e($i)"
}

proc sensePower {} {
	global array names n ns array names val array names e
	set t [$ns now]
	for {set i 1} {$i<$val(nn)} {incr i} {
		set e($i) [expr $e($i)-0.0175]
		 # $ns at $t "$n($i) label $e($i)"
	}
	$ns at [expr $t+0.5] "sensePower"
}

proc transPower { b } {
	global ns array names n nn array names e rt re nn 
	set t [$ns now]
		set e($b) [expr $e($b)-0.9]
		# $ns at $t "$n($b) label $e($b)"
}

proc receivePower { ma } {
	global ns array names n nn array names e rt re nn 
	set t [$ns now]
		set e($ma) [expr $e($ma)-0.8]
		# $ns at $t "$n($ma) label $e($ma)"
}

set ch1 [list 5 6 7 8 9 10 11 12 13 14 15] 

set ch2 [list 16 17 18 19 20 21 22 23 24 25 26]

set ch3 [list 27 28 29 30 31 32 33 34 35 36]

set ch4 [list 37 38 39 40 41 42 43 44 45 46 47 48 49]


set ch_len [llength $ch1]
set ch_len2 [llength $ch2]
set ch_len3 [llength $ch3]
set ch_len4 [llength $ch4]
set ch_len5 [llength $ch5]
set ch_len6 [llength $ch6]
set ch_len7 [llength $ch7]
set ch_len8 [llength $ch8]
set ch_len9 [llength $ch9]
#######################################################
proc random_int { upper_limit } {

	global myrand
        set myrand [expr int(rand() * $upper_limit + 1)]
        return $myrand

}

proc prim { args } {

	set m 0

	set vv [lindex $args 0]

	set eculer [lindex $args 1]

	############################################

	if { $vv > $eculer } {

		set m $eculer

	} else {

		set m $vv	

	}

	set re_value 0

	for { set i 2 } { $i <= $m } { incr i } {

		set mod1 [ expr $vv % $i ]

		set mod2 [ expr $eculer % $i ]

		if { $mod1 == 0 && $mod2 == 0 } {

			#puts "ii : $i"

			set re_value $i

			break
		}

	}

	

	if { $i > $m && $re_value == 0 } {

		set re_value 1
		
		
	}

	return $re_value

	###########################################

	###########################################

}	

proc powm {n r m} {
          set myrand [expr {$r & 1 ? $n * [powm $n [expr {$r-1}] $m] % $m : $r ? [set nn [powm $n [expr {$r >> 1}] $m]] * $nn % $m : 1 }]
return $myrand

}


proc En_calc {tx_dr Rx_dr p_s N_type N_node } {
global  tx_power Rx_power Count_time Curr_Energy
set tx_rx_time 0.001
set tx_data_rate [expr [lindex [split $tx_dr "M"] 0]*1000000]
#puts $Rx_dr
set Rx_data_rate [expr [lindex [split $Rx_dr "M"] 0]*1000000]
set Pkt_size [expr $p_s*1000] 
if {$N_type == 1 } {

set n_tx_pkts [expr $tx_data_rate/$Pkt_size]
set n_Rx_pkts [expr $Rx_data_rate/$Pkt_size]
set Tx_decr_power [expr $n_tx_pkts*$tx_power]
set Rx_decr_power [expr $n_Rx_pkts*$Rx_power*($N_node-1)]
set Curr_Energy [expr $Curr_Energy-(($Tx_decr_power*$tx_rx_time + $Rx_decr_power*$tx_rx_time)*$Count_time)]
} else { 
set n_tx_pkts [expr $tx_data_rate/$Pkt_size]
set Tx_decr_power [expr $n_tx_pkts*$tx_power]
set Curr_Energy [expr $Curr_Energy-($Tx_decr_power*$tx_rx_time*$Count_time)] 
}
set Curr_Energy [expr $Curr_Energy]


#puts "==========================>$Curr_Energy"
return En_calc

}
for {set v 0} {$v<100} { incr v} {
set node_current_energy($v) 10
}

set tx_power 1
set Rx_power 0.5
set Count_time 2

#En_calc 500000 200000 100000 0 5 ;#100
#
set nt 0.5
set nt1 1.0
proc random {list} {
	set index [expr { int ( 1+ rand() * ([llength $list])-1)}]
	return $index
	puts "$index"
	}


after 500



set tim1 1.0

set cr1 10
set in [random $ch1]
	set n1 [lindex $ch1 $in]
$ns at [expr $nt+$nt1] "$n($n1) label CH"
		puts "CH node $n($n1)"


	set in2 [random $ch2]
	set n2 [lindex $ch2 $in2]
$ns at [expr $nt+$nt1] "$n($n2) label CH"


	set in3 [random $ch3]
	set n3 [lindex $ch3 $in3]
$ns at [expr $nt+$nt1] "$n($n3) label CH"


	set in4 [random $ch4]
	set n4 [lindex $ch4 $in4]
$ns at [expr $nt+$nt1] "$n($n4) label CH"


set x_pos1 [$n($n1) set X_]
	set y_pos1 [$n($n1) set Y_]
	for {set j 0} {$j<$val(nn)} {incr j} {
			set x_pos2 [$n($j) set X_]
			set y_pos2 [$n($j) set Y_]
			set x_pos [expr $x_pos1-$x_pos2]
			set y_pos [expr $y_pos1-$y_pos2]
			set v [expr $x_pos*$x_pos+$y_pos*$y_pos]
			set d [expr sqrt($v)]
			puts "Distance from n($n1) to $j:$d"
			if {$d<$cr1} {
				$n($n1) add-neighbor $n($j)
			}
		
	}

	set neighbor1 [$n($n1) neighbors]
	foreach nb1 $neighbor1 {
		set now [$ns now]
		puts "The neighbor for node n($n1) are:$nb1"
		set idv [$nb1 id]
		puts "$idv"	
		lappend NL($i) $idv
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
	$ns connect $source $sink
	return $traffic
}
set nulla [new Agent/LossMonitor]

# Transmission from nodes to CH
	$ns attach-agent $n(1) $nulla
	set cbr1 [attach-cbr-traffic $n($n1) $nulla 512 0.05]
	$ns at $tim1 "$cbr1 start"
	$ns at [expr $tim1+$nt1+3] "$cbr1 stop"
	$ns at $tim1 "$ns trace-annotate \" Transmission from the cluster head n($n1) to BS \""
	$ns at [expr $tim1+0.5] "$ns trace-annotate \" The CH is n($n1) \""
	$ns at $tim1 "$n($n1) add-mark R red"
	$ns at [expr $tim1+$nt1+3]  "$n($n1) delete-mark R"

set nulla1 [new Agent/LossMonitor]

	$ns attach-agent $n(2) $nulla1
	set cbr1 [attach-cbr-traffic $n($n2) $nulla1 512 0.05]
	$ns at $tim1 "$cbr1 start"
	$ns at [expr $tim1+$nt1+3] "$cbr1 stop"
	$ns at $tim1 "$ns trace-annotate \" Transmission from the cluster head n($n2)  to BS \""
	$ns at [expr $tim1+0.5] "$ns trace-annotate \" The CH node is n($n2) \""
	$ns at $tim1 "$n($n2) add-mark R red"
	$ns at [expr $tim1+$nt1+3]  "$n($n2) delete-mark R"


set nulla [new Agent/LossMonitor]

# Transmission from nodes to CH
	$ns attach-agent $n(3) $nulla
	set cbr1 [attach-cbr-traffic $n($n3) $nulla 512 0.05]
	$ns at $tim1 "$cbr1 start"
	$ns at [expr $tim1+$nt1+3] "$cbr1 stop"
	$ns at $tim1 "$ns trace-annotate \" Transmission from the CH n($n3) to BS\""
	$ns at [expr $tim1+0.5] "$ns trace-annotate \" The CH is n($n3) \""
	$ns at $tim1 "$n($n3) add-mark R red"
	$ns at [expr $tim1+$nt1+3]  "$n($n3) delete-mark R"

set nulla1 [new Agent/LossMonitor]

	$ns attach-agent $n(4) $nulla1
	set cbr1 [attach-cbr-traffic $n($n4) $nulla1 512 0.05]
	$ns at $tim1 "$cbr1 start"
	$ns at [expr $tim1+$nt1+3] "$cbr1 stop"
	$ns at $tim1 "$ns trace-annotate \" Transmission from the n($n4) to BS\""
	$ns at [expr $tim1+0.5] "$ns trace-annotate \" The CH node is n($n4) \""
	$ns at $tim1 "$n($n4) add-mark R red"
	$ns at [expr $tim1+$nt1+3]  "$n($n4) delete-mark R"


set nulla [new Agent/LossMonitor]

# Transmission from nodes to CH
	$ns attach-agent $n(0) $nulla
	set cbr1 [attach-cbr-traffic $n(1) $nulla 512 0.05]
	$ns at $tim1 "$cbr1 start"
	$ns at [expr $tim1+$nt1+3] "$cbr1 stop"
	$ns at $tim1 "$ns trace-annotate \" Transmission from the CH n($n5) to BS\""
	$ns at [expr $tim1+0.5] "$ns trace-annotate \" The CH is n($n5) \""
	$ns at $tim1 "$n($n5) add-mark R red"
	$ns at [expr $tim1+$nt1+3]  "$n($n5) delete-mark R"

set nulla1 [new Agent/LossMonitor]

	$ns attach-agent $n(0) $nulla1
	set cbr1 [attach-cbr-traffic $n(2) $nulla1 512 0.05]
	$ns at $tim1 "$cbr1 start"
	$ns at [expr $tim1+$nt1+3] "$cbr1 stop"
	$ns at $tim1 "$ns trace-annotate \" Transmission from the n($n6) to BS\""
	$ns at [expr $tim1+0.5] "$ns trace-annotate \" The CH node is n($n6) \""
	$ns at $tim1 "$n($n6) add-mark R red"
	$ns at [expr $tim1+$nt1+3]  "$n($n6) delete-mark R"


set nulla1 [new Agent/LossMonitor]

	$ns attach-agent $n(0) $nulla1
	set cbr1 [attach-cbr-traffic $n(3) $nulla1 512 0.05]
	$ns at $tim1 "$cbr1 start"
	$ns at [expr $tim1+$nt1+3] "$cbr1 stop"
	$ns at $tim1 "$ns trace-annotate \" Transmission from the n($n7) to BS\""
	$ns at [expr $tim1+0.5] "$ns trace-annotate \" The CH node is n($n7) \""
	$ns at $tim1 "$n($n7) add-mark R red"
	$ns at [expr $tim1+$nt1+3]  "$n($n7) delete-mark R"


set nulla [new Agent/LossMonitor]

# Transmission from nodes to CH
	$ns attach-agent $n(0) $nulla
	set cbr1 [attach-cbr-traffic $n(4) $nulla 512 0.05]
	$ns at $tim1 "$cbr1 start"
	$ns at [expr $tim1+$nt1+3] "$cbr1 stop"
	$ns at $tim1 "$ns trace-annotate \" Transmission from the CH n($n8) to BS\""
	$ns at [expr $tim1+0.5] "$ns trace-annotate \" The CH is n($n8) \""
	$ns at $tim1 "$n($n8) add-mark R red"
	$ns at [expr $tim1+$nt1+3]  "$n($n8) delete-mark R"


#$ns at 1.0 "source cbr.tcl"
#$ns at 1.0 "source cbr1.tcl"
#$ns at 1.0 "source cbr2.tcl"
#Define a 'finish' procedure
proc finish {} {
    global ns tracefile namfile
    $ns flush-trace
    close $tracefile
    close $namfile
    exec nam out.nam &
    exit 0
}
for {set i 0} {$i < $val(nn) } { incr i } {
    $ns at $val(stop) "\$n($i) reset"
}
$ns at $val(stop) "$ns nam-end-wireless $val(stop)"
$ns at $val(stop) "finish"
$ns at $val(stop) "puts \"done\" ; $ns halt"
$ns run