import React from "react";
import IconLogo from "../Svg/IconLogo";
import {routing} from "../routes/routing";
import Link from "next/link";
import site from "../constants/siteConstants";
import Icon from "../Svg/Icon";
import {mk} from "../utils/mk";


const Footer = ({is_index, styles, imgParams}) => {
    let footer_btn_id = ''
    let footer_link_contacts = ''
    let footer_link_about = ''
    let footer_link_services = ''
    let footer_link_portfolio = ''
    let footer_href_prefix = ''
    if (is_index) {
        footer_btn_id = 'indexFooterContact'
        footer_link_contacts = 'indexFLinkContacts'
        footer_link_about = 'indexFLinkAbout'
        footer_link_services = 'indexFLinkServices'
        footer_link_portfolio = 'indexFLinkPortfolio'
        footer_href_prefix = ''
    }
    else {
        footer_btn_id = 'internalFooterContact'
        footer_link_contacts = 'internalFLinkContacts'
        footer_link_services = 'internalFLinkServices'
        footer_link_about = 'internalFLinkAbout'
        footer_link_portfolio = 'internalFLinkPortfolio'
        footer_href_prefix = '/'

    }

    return (
        <footer className={mk("container-fluid footer", styles)}>
            <div className={styles["container"]}>
                <div className={styles["row"]}>
                    <div className={styles["col-sm"]}>
                        <div className={styles["row"]}>
                            <div className={styles["col-md"]}>
                                <Link href="/" passHref>
                                    <a className={mk("navbar-brand align-top", styles)}>
                                        <IconLogo icon='logo-innostan'
                                                  style={{fill: "#fcfcfc", verticalAlign: "top" }}
                                                  file='main'
                                                  file_noscrpit={site.data.nav.logo.footer}
                                                  title="На главную"
                                                  alt="Станкостроительный завод ИнноСтан"
                                                  styles={styles}
                                                  imgParams={imgParams}
                                        />
                                    </a>
                                </Link>
                                <p className={styles["btn-p"]} itemScope itemType="http://schema.org/CommunicateAction">
                                    <Link href={`${footer_href_prefix}#${site.data.nav.write_us.anchor_}`} passHref scroll={false}>
                                        <a id={footer_btn_id} className={mk("btn btn-primary", styles)} role="button">
                                            <span className="wr">Отправить заявку</span>
                                        </a>
                                    </Link>
                                </p>
                                <p className={styles["v2"]}>Инновационное станкостроение</p>
                                <p className={styles["v1"]}>
                                    <span className={styles["no-break-nobr"]}>&copy; ООО &laquo;ИнноСтан&raquo;</span>,{" "}
                                    {site.data.config.currentYear}
                                </p>
                            </div>
                            <div className={styles["col-md"]}>
                                <address>
                                    <p className={mk("phone1 phone", styles)}>
                                        <span className={styles["phone-icon"]}>
                                            <Icon icon="phone"
                                                  className={styles["ic-phone"]}
                                                  file='main'
                                                  styles={styles}
                                                  imgParams={imgParams}
                                            />
                                        </span>
                                        <Link href={`tel:${ site.data.organization.phone1_href }`} passHref scroll={false}>
                                            <a className={styles["phone-ref"]}>{site.data.organization.phone1}</a>
                                        </Link>
                                    </p>
                                    <p className={styles["email"]}>
                                        <span className={styles["email-icon"]}>
                                            <Icon icon="mail"
                                                  className={styles["ic-mail"]}
                                                  file='main'
                                                  styles={styles}
                                                  imgParams={imgParams}
                                            />
                                        </span>
                                        <span className={styles["wr"]}>
                                            <Link href={`mailto:${ site.data.organization.email }`} passHref scroll={false}>
                                                <a className={styles["email-ref"]}>{ site.data.organization.email }</a>
                                            </Link>
                                        </span>
                                    </p>
                                    <Link href={`${footer_href_prefix}}#${site.data.nav.contacts.anchor}`} passHref scroll={false}>
                                        <a className={styles["contacts"]}
                                           id={ footer_link_contacts }
                                           role='button'
                                           aria-haspopup='true'
                                        >
                                            <span className={styles["wr"]}>Контактная информация</span>
                                        </a>
                                    </Link>
                                </address>
                            </div>
                        </div>
                    </div>
                    <div className={styles["col-sm"]}>
                        <nav>
                            <div className={styles["row"]}>
                                <div className={styles["col-md"]} role='menubar'>
                                    <ul className={styles["footer-service-menu"]} aria-expanded="false">
                                        <li role="menuitem" className={styles["footer-nav-item service-main"]}>
                                            <Link href={`${footer_href_prefix}#${site.data.nav.service_main.anchor}`} passHref scroll={false}>
                                            <a id={ footer_link_services }
                                               className={mk("footer-nav-link noservice", styles)}
                                               role='button'
                                               aria-haspopup='true'>
                                                <span className={styles["wr"]}>{ site.data.nav.service_main.title }</span>
                                            </a>
                                            </Link>
                                        </li>
                                        {
                                            site.data.nav.services.map(
                                                (service, index) => (
                                                    <li role="menuitem" className={styles["footer-nav-item"]} key={index}>
                                                        <Link href={service.url ? service.url : undefined} passHref>
                                                            <a className={mk("footer-nav-link service", styles)}>
                                                                <span className={styles["wr"]} dangerouslySetInnerHTML={{__html:service.title_menu}}></span>
                                                            </a>
                                                        </Link>
                                                    </li>
                                                )
                                            )
                                        }
                                    </ul>
                                </div>
                                <div className={styles["col-md"]}>
                                    <Link href={`${ footer_href_prefix }}#${site.data.nav.about.anchor_}`} passHref scroll={false}>
                                        <a id={footer_link_about}
                                           className={mk("footer-nav-link noservice", styles)}
                                           role='button'
                                           aria-haspopup='true'
                                        >
                                            <span className={styles["wr"]}>О компании</span>
                                        </a>
                                    </Link>
                                    <Link href={`${footer_href_prefix}}#${site.data.nav.portfolio.anchor_}`} passHref scroll={false}>
                                        <a id={footer_link_portfolio}
                                           className={mk("footer-nav-link noservice", styles)}
                                           role='button'
                                           aria-haspopup='true'
                                        >
                                            <span className={styles["wr"]}>
                                                <span className={styles["no-break-nobr"]}>Выполненные проекты</span>
                                            </span>
                                        </a>
                                    </Link>
                                </div>
                            </div>
                        </nav>
                    </div>
                </div>
            </div>
        </footer>
    );
}
export default Footer;