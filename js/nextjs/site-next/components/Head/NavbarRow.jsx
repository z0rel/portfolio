import React, {useEffect, useRef, useState} from "react";
import IconLogo from "../Svg/IconLogo";
import Link from "next/link";
import site from "../constants/siteConstants";
import {mk} from "../utils/mk";

let NavbarRow = ({is_index, nav_marker, internal, imgParams, styles}) => {

    let [opened, setOpened] = useState(false);
    let [transition, setTransition] = useState(false);
    let buttonRef = useRef(null);

        let header_link_contacts = ''
    let header_link_about = ''
    let header_link_services = ''
    let header_link_portfolio = ''
    let header_href_prefix = ''
    if (is_index) {
        header_link_contacts  = 'indexHLinkContacts'
        header_link_about     = 'indexHLinkAbout'
        header_link_services  = 'indexHLinkServices'
        header_link_portfolio = 'indexHLinkPortfolio'
        header_href_prefix    = ''
    }
    else {
        header_link_contacts  = 'internalHLinkContacts'
        header_link_services  = 'internalHLinkServices'
        header_link_about     = 'internalHLinkAbout'
        header_link_portfolio = 'internalHLinkPortfolio'
        header_href_prefix    = '/'
    }

    const modalHandler = (event) => {
        let val = buttonRef.current.ariaExpanded;
        let body = document.querySelector('body')
        // console.log('Handle click', val, buttonRef, event)
        if (!transition) {
            if (!val || val == 'false') {
                body.classList.add(styles["modal-open"])
                setOpened(true);
                setTransition(true);
            } else {
                body.classList.remove(styles["modal-open"])
                setOpened(false);
                setTransition(true);
            }
        }
    }

    useEffect(() => {
        setTimeout(() => setTransition(false), 200)
    }, [opened])

    const getCollapsingClass = () => {
        if (transition) {
            return 'collapsing';
        }
        else if (opened) {
            return 'collapse show'
        }
        else {
            return 'collapse'
        }
    }

    const getButtonClass = () => {
        let str = mk(`navbar-toggler ${opened ? '' : 'collapsed'}`, styles);
        // console.log(str, opened);
        return str;
    }

    // console.log(styles);
    return <div className={styles.row}>
        <div className={mk("col-12 customNavContainer", styles)}>
            <nav className={mk("navbar navbar-expand-md", styles)}>
                <button className={getButtonClass()}
                        id={styles.navbarBtn}
                        type="button"
                        data-toggle="collapse"
                        data-target={`#${styles.navbarNavigationMenu}`}
                        aria-controls={styles.navbarNavigationMenu}
                        aria-expanded={opened ? "true": "false"}
                        aria-label="Toggle navigation"
                        onClick={modalHandler}
                        ref={buttonRef}
                >
                    <span className={mk("lay one", styles)}>&nbsp;</span>
                    <span className={mk("lay two", styles)}>&nbsp;</span>
                    <span className={mk("lay three", styles)}>&nbsp;</span>
                </button>
                <div
                    className={mk(`navbar-collapse navbar-restricted-area ${getCollapsingClass()}`, styles)}
                    id={styles.navbarNavigationMenu}>
                    <ul className={styles["navbar-nav"]}
                        itemScope itemType="http://www.schema.org/SiteNavigationElement"
                        role="menubar">
                        <li role="menuitem"
                            className={mk("nav-item not-services", styles)}>
                            <span className={styles.wr}>
                                <Link href={`${ header_href_prefix }#${ site.data.nav.about.anchor_ }`}
                                      passHref
                                      scroll={false}
                                >
                                    <a id={header_link_about}
                                       className={mk("nav-link noservice", styles)}
                                       dangerouslySetInnerHTML={{__html: site.data.nav.about.title}}></a>
                                </Link>
                            </span>
                        </li>
                        <li role="menuitem" className={mk("nav-item not-services services-main", styles)}>
                            <span className={styles.wr}>
                                <Link href={`${ header_href_prefix }#${ site.data.nav.service_main.anchor_ }`}
                                      scroll={false}
                                      passHref
                                >
                                    <a id={header_link_services}
                                       className={mk("nav-link noservice", styles)}
                                       dangerouslySetInnerHTML={{__html: site.data.nav.service_main.title}}></a>
                                </Link>
                            </span>
                        </li>
                        {
                            site.data.nav.services.map(
                                (service, index) => (
                                    <li role="menuitem" className={mk("nav-item not-services", styles)} key={index}>
                                        {nav_marker == service.nav_marker
                                            ? (
                                                <span className={styles["strong-wr"]}>
                                                    <strong>{service.title_menu}</strong>
                                                </span>
                                            )
                                            : service.url
                                                ? (<span className={styles.wr}>
                                                    <Link href={service.url} passHref>
                                                        <a className={mk("nav-link service", styles)}
                                                           dangerouslySetInnerHTML={{__html: service.title_menu}}></a>
                                                    </Link>
                                                  </span>)
                                                : (<span className={styles.wr}>
                                                    <a className={mk("nav-link service", styles)}
                                                       dangerouslySetInnerHTML={{__html: service.title_menu}}>
                                                    </a>
                                                  </span>)
                                        }
                                    </li>
                                )
                            )
                        }
                        <li className={styles["menu-divider"]} />
                        <li role="menuitem" className={mk("nav-item not-services", styles)}>
                            <span className={styles.wr}>
                                <Link href={`${ header_href_prefix }#${ site.data.nav.portfolio.anchor_ }`}
                                      passHref
                                      scroll={false}
                                >
                                    <a id={ header_link_portfolio }
                                       className={mk("nav-link noservice", styles)}
                                       dangerouslySetInnerHTML={{__html: site.data.nav.portfolio.title}}></a>
                                </Link>
                            </span>
                        </li>
                        <li role="menuitem" className={mk("nav-item not-services", styles)}>
                            <span className={styles.wr}>
                                <Link href={`${ header_href_prefix }#${ site.data.nav.contacts.anchor_ }`}
                                      passHref
                                      scroll={false}
                                >
                                    <a id={ header_link_contacts }
                                       className={mk("nav-link noservice", styles)}
                                       dangerouslySetInnerHTML={{__html: site.data.nav.contacts.title}}></a>
                                </Link>
                            </span>
                        </li>
                    </ul>
                </div>
                <Link href="/" passHref>
                    <a className={mk("navbar-brand align-top", styles)} >
                        {
                            internal
                                ? <IconLogo icon='logo-innostan-internal'
                                            style={{fill: '#4eaacf', verticalAlign: 'top'}}
                                            className={styles['logo-internal']}
                                            file='internal'
                                            imgParams={imgParams}
                                            specIconFname={site.data.nav.logo.internal}
                                            title="На главную"
                                            alt="Станкостроительный завод ИнноСтан"
                                            styles={styles}
                                />
                                : <></>
                        }
                        <IconLogo icon='logo-innostan'
                                  style={{fill: '#fcfcfc', verticalAlign: 'top'}}
                                  className={styles['logo-main']}
                                  file='main'
                                  imgParams={imgParams}
                                  specIconFname={site.data.nav.logo.footer}
                                  title="На главную"
                                  alt="Станкостроительный завод ИнноСтан"
                                  styles={styles}
                        />
                    </a>
                </Link>
            </nav>
        </div>
    </div>
}

export default NavbarRow;