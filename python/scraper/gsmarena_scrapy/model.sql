CREATE TABLE IF NOT EXISTS vendors
(
    vendor_name   VARCHAR(200),
    vendor_source VARCHAR(100),
    devices_cnt   BIGINT,
    group_url     VARCHAR(750)
);

-- =</>= 

ALTER TABLE vendors
    ADD PRIMARY KEY (vendor_name, vendor_source);

-- =</>= 

CREATE TABLE IF NOT EXISTS gsmarena_phones
(
    vendor_name               VARCHAR(200),
    vendor_source             VARCHAR(100),
    phone_title               VARCHAR(300),
    phone_brief               TEXT,
    phone_url                 VARCHAR(750),
    fullname                  VARCHAR(500),
    main_img_url              VARCHAR(750), 
    become_a_fan              INT,
    help_popularity           BIGINT,
    released_date             VARCHAR(150),  
    body_hl                   VARCHAR(200),  
    os_hl                     VARCHAR(300),  
    storage                   VARCHAR(300),
    modelname                 VARCHAR(300),
    displaysize               VARCHAR(50),
    display_res               VARCHAR(100), 
    camerapixels              VARCHAR(50),
    videopixels               VARCHAR(100),
    ramsize_gb                VARCHAR(50),
    chipset                   VARCHAR(200),
    batsize                   VARCHAR(100),
    battype                   VARCHAR(100),
    comment                   TEXT, 
    net_tech                  VARCHAR(100),
    net_edge                  VARCHAR(100),
    net_gprs                  VARCHAR(100),
    net_2g                    VARCHAR(200),
    net_3g                    VARCHAR(100),
    net_4g                    VARCHAR(300),
    net_speed                 VARCHAR(200),
    net_nfc                   VARCHAR(100),
    announced                 VARCHAR(100),
    phone_status              VARCHAR(150),
    body_dimensions           VARCHAR(100),
    body_weight               VARCHAR(100),
    body_sim                  VARCHAR(100),
    body_other                VARCHAR(400),
    body_build                VARCHAR(200),
    display_type              VARCHAR(100),
    display_size              VARCHAR(100),
    display_resolution        VARCHAR(100),
    display_protection        VARCHAR(100),
    display_other             VARCHAR(300),
    platform_os               VARCHAR(200),
    platform_chipset          VARCHAR(100),
    platform_cpu              VARCHAR(100),
    platform_gpu              VARCHAR(100),
    memory_card_slot          VARCHAR(100),
    memory_internal           VARCHAR(100),
    memory_other              VARCHAR(100),
    main_camera_single        VARCHAR(100),
    main_camera_features      VARCHAR(100),
    main_camera_video         VARCHAR(100),
    selfie_camera_single      VARCHAR(100),
    selfie_camera_features    VARCHAR(100),
    selfie_camera_video       VARCHAR(100),
    sound_loudspeaker         VARCHAR(50),
    sound_35_jack             VARCHAR(50),
    optional_other            VARCHAR(200),
    wlan                      VARCHAR(200),
    bluetooth                 VARCHAR(100),
    gps                       VARCHAR(100),
    radio                     VARCHAR(100),
    usb                       VARCHAR(100),
    sensors                   VARCHAR(300),
    features_other            TEXT,
    battery_descr             VARCHAR(100),
    battery_talk_time         VARCHAR(100),
    battery_standby           VARCHAR(100),
    battary_music_play        VARCHAR(50),
    misc_colors               VARCHAR(300),
    misc_sar_eu               VARCHAR(50),
    misc_sar_us               VARCHAR(50),
    price                     VARCHAR(100),
    test_performance          VARCHAR(200),
    test_battary_life         VARCHAR(100),
    test_display              VARCHAR(100),
    test_camera               VARCHAR(50),
    test_loudspeaker          VARCHAR(100),
    test_audio_quality        VARCHAR(50),
    comment_start_url         VARCHAR(750)
);

-- =</>= 

ALTER TABLE gsmarena_phones
    ADD PRIMARY KEY (vendor_name, vendor_source, phone_title);

-- =</>= 

CREATE TABLE IF NOT EXISTS gsmarena_comments
(
    post_id               BIGINT,
    vendor_name           VARCHAR(200),
    vendor_source         VARCHAR(100),
    phone_title           VARCHAR(300),
    phone_url             VARCHAR(750),
    user_name             VARCHAR(500),
    user_location         VARCHAR(100),
    user_location_type    VARCHAR(500),
    user_post_date        DATE,
    user_post_rating      INT,
    user_post_text        TEXT,
    user_reply_name       VARCHAR(500),
    user_reply_date       DATE,
    user_reply_msg        TEXT,
    user_reply_msg_id     BIGINT,
    user_post_count       BIGINT,
    user_upvote_count     BIGINT
);

-- =</>= 

ALTER TABLE gsmarena_comments 
    ADD PRIMARY KEY (post_id, vendor_name, phone_title);

-- =</>= 

CREATE TABLE IF NOT EXISTS scrap_comments_status 
(
    vendor_name   VARCHAR(200),
    vendor_source VARCHAR(100),
    phone_title   VARCHAR(300),
    phone_url     VARCHAR(750),
    scrapped      BOOLEAN
);

-- =</>= 

ALTER TABLE scrap_comments_status
    ADD PRIMARY KEY (vendor_name, vendor_source, phone_title);


-- =</>= 

CREATE TABLE IF NOT EXISTS scrap_vendor_status 
(
    vendor_name   VARCHAR(200),
    vendor_source VARCHAR(100),
    scrapped      BOOLEAN
)

-- =</>= 

ALTER TABLE scrap_vendor_status
    ADD PRIMARY KEY (vendor_name, vendor_source);
