@echo ON
fab deploy-dotenv
fab redeploy-whl
fab deploy-migrate
REM fab deploy-load-data-to-db --fixtures=load_um_locations,load_um_constructions_rts,load_um_constructions_nonrts,load_partners
fab deploy-static-assets
fab deploy-systemd-service
fab set-maximum-numbers
fab distclean
REM exit