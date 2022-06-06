
-- = </>= 

ALTER TABLE gsmarena_comments 
    ADD FOREIGN KEY(vendor_name, vendor_source, phone_title) 
         REFERENCES gsmarena_phones (vendor_name, vendor_source, phone_title) ON DELETE RESTRICT ON UPDATE RESTRICT;

-- = </>= 

ALTER TABLE gsmarena_comments 
    ADD FOREIGN KEY (phone_url) REFERENCES gsmarena_phones (phone_url) ON DELETE RESTRICT ON UPDATE RESTRICT;
