# Makefile for ...su/main

include $(CWPROOT)/src/Makefile.config

D = $L/libcwp.a $L/libpar.a $L/libsu.a 
LFLAGS= -I/u/000000/include -L$L -lsu -lpar -lcwp -lm -L/u/000000/lib64 -lgmt -lpsl -lnetcdf 

PROGS =			  \
	$B/joebob-vzerok  \
	$B/joebob-window-vzerok \
	$B/horizon-single-vzerok \
	$B/joebob-vzerok-triple \
	$B/joebob-vzerok-double \
	$B/horizon-joebob-vzerok-triple \
	$B/joebob-vzerok-poly-hybrid \
	$B/joebob-vzerok-vavg-vzero \
	$B/insert-campanian-surfaces \
	$B/joebob-vzerok-vavg \
	$B/inverse-horizon-vzerok \
	$B/suvzerok_with_water_seis_vavg_horizon \
	$B/supoly_with_water_seis_vavg_horizon \
	$B/sutest.vzerok.simulate \
	$B/sutest.vzerok.checkshots.simulate \
	$B/match.wmp.xy.to.deltav \
	$B/solveit \
	$B/extract_delta_v \
	$B/add_delta_v \
	$B/apply_delta_v \
	$B/apply_delta_v.new \
	$B/apply_deltav_v1 \
	$B/apply_deltav_revised \
	$B/apply_delta_v_check \
	$B/fix_xy_geometry_su_trace_headers \
	$B/intersect_checkshot_with_grid \
	$B/supoly \
	$B/sum_data \
	$B/sum_data_t \
	$B/sum_decimate_deltat \
	$B/update.average.velocities \
	$B/sudepthconvert.v3 \
	$B/sudepthconvert.test \
	$B/sudepthconvert.simple \
	$B/apply_average_velocity \
	$B/suextract.traverse.path \
	$B/supoly.scale.adjust.velocities \
	$B/supoly.scale.adjust.velocities.v1 \
	$B/supoly.scale.adjust.velocities.GRIDS \
	$B/make.sure.average.vels.increase \
	$B/final_corrections \
	$B/vertical_final_corrections \
	$B/vertical_final_corrections_PARALLEL \
	$B/horizon-vzerok 

INSTALL	:	$(PROGS)
	touch $@


$(PROGS):	$(CTARGET) $D 
	-$(CC) -fopenmp $(CFLAGS) $(@F).c $(LFLAGS) -o $@
	@chmod 755 $@
	@echo $(@F) installed in $B

remake	:
	-touch *.c 
	-rm -f $(PROGS)
	$(MAKE) 
	
clean:
	rm -f a.out junk* JUNK* core
