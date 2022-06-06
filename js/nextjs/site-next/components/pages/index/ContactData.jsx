import {mk} from "../../utils/mk";
import site from "../../constants/siteConstants";
import IconAlt from "../../Svg/IconAlt";
import {routing} from "../../routes/routing";
import Link from "next/link";


const ContactsData = ({styles, imgParams}) => {
    return (
        <section className={mk("container-fluid cdt", styles)}>
            <div className={styles["container"]}>
                <h1 id={styles[site.data.nav.contacts.anchor_]}>Контакты</h1>
                <address>
                    <div className={styles["address"]}>
                        <p className={styles["phone1"]}>
                            <span className={styles["recv-head"]}>Телефон</span>
                            <span className={styles["contact-val"]}>
                                <span className={styles["phone-icon"]}>
                                    <IconAlt className="ic-phone"
                                             icon="phone"
                                             icon_alt="phone-b"
                                             file='main'
                                             styles={styles}
                                             imgParams={imgParams}
                                    />
                                </span>
                                <span className={styles["val"]} itemProp="telephone">
                                    <Link href={`tel:${ site.data.organization.phone1_href }`} passHref scroll={false}>
                                        <a>{ site.data.organization.phone1 }</a>
                                    </Link>
                                </span>
                            </span>
                        </p>
                        <p className={styles["email"]}>
                            <span className={styles["recv-head"]}>Электронная почта</span>
                            <span className={styles["contact-val"]}>
                                <span className={styles["email-icon"]}>
                                    <IconAlt className={styles["ic-mail"]}
                                             icon="mail"
                                             icon_alt="mail-b"
                                             file='main'
                                             styles={styles}
                                             imgParams={imgParams}
                                    />
                                </span>
                                <span className={styles["val"]}>
                                    <span className={styles["wr"]}>
                                        <Link href={`mailto:${ site.data.organization.email }`} passHref scroll={false}>
                                            <a>
                                                <span itemProp="email">{ site.data.organization.email }</span>
                                            </a>
                                        </Link>
                                    </span>
                                </span>
                            </span>
                        </p>
                        <p className={styles["address-val"]}>
                            <span className={styles["recv-head"]}>Адрес</span>
                            <span className={styles["addr-val"]} itemProp="address" itemScope itemType="http://schema.org/PostalAddress">
                                <span itemProp="postalCode">394068</span>,{" "}
                                <span itemProp="addressCountry">Россия</span>,{" "}
                                <span itemProp="addressRegion">Воронежская область</span>,{" "}
                                г.&nbsp;<span itemProp="addressLocality">Воронеж</span>,{" "}
                                <span itemProp="streetAddress">ул.&nbsp;Московский проспект, дом&nbsp;112, помещение&nbsp;310</span>.
                            </span>
                        </p>
                        <p className={styles["INNKPP"]}>
                            <span className={styles["recv-head"]}>ИНН / КПП </span>
                            <span className={styles["val"]}>
                                <span itemProp={styles["vatID"]}>{ site.data.organization.inn } </span>
                                / { site.data.organization.kpp }
                            </span>
                        </p>
                        <p className={styles["INN"]}>
                            <span className={styles["recv-head"]}>ИНН </span>
                            <span className={styles["val"]} itemProp="taxID">{ site.data.organization.inn }</span>
                        </p>
                        <p className={styles["KPP"]}>
                            <span className={styles["recv-head"]}>КПП </span>
                            <span className="val">{ site.data.organization.kpp }</span>
                        </p>
                        <p className={styles["OGRN"]}>
                            <span className={styles["recv-head"]}>ОГРН </span>
                            <span className={styles["val"]}>{ site.data.organization.ogrn }</span>
                        </p>
                    </div>
                </address>
            </div>
        </section>
    )
}
export default ContactsData;