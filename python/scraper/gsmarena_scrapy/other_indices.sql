-- =</>= 

ALTER TABLE gsmarena_phones
    ADD UNIQUE KEY (phone_url);

-- =</>= 

ALTER TABLE scrap_comments_status
    ADD UNIQUE KEY (phone_URL);

