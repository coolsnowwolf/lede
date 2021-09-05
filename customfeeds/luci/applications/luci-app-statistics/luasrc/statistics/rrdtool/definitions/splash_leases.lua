-- Copyright 2013 Freifunk Augsburg / Michael Wendland <michael@michiwend.com>
-- Licensed to the public under the Apache License 2.0.

    
module("luci.statistics.rrdtool.definitions.splash_leases", package.seeall)
                                       
function rrdargs( graph, plugin, plugin_instance, dtype )
                             
    return {
        title = "%H: Splash Leases",
        vlabel = "Active Clients",  
        y_min = "0",          
        number_format = "%5.1lf",
        data = {         
            sources = {
                splash_leases = { "leased", "whitelisted", "blacklisted" }
            },                                
              
            options = {
                splash_leases__leased      = { color = "00CC00", title = "Leased",      overlay = false },
                splash_leases__whitelisted = { color = "0000FF", title = "Whitelisted", overlay = false },
                splash_leases__blacklisted = { color = "FF0000", title = "Blacklisted", overlay = false } 
            }                                                 
        }    
    }

end 
