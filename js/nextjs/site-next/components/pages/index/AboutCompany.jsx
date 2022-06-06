import React from "react";
import {mk} from "../../utils/mk";
import site from "../../constants/siteConstants";
import microdata from "../../microdata/microdata";
import {routing} from "../../routes/routing";
import Link from "next/link";
import Icon from "../../Svg/Icon";

const AboutCompany = ({styles, imgParams}) => {
	const LiItem = ({liClass, iconName, iconClass, url, children}) => {
		if (!iconClass) {
			iconClass = iconName
		}

		return (
			<li className={styles[liClass]} {...microdata.ITEM_LIST_OFFER}>
				<Icon spanCover={styles["li-cover"]}
					  className={mk(`icon-our-work s-${iconClass}`, styles)}
					  icon={iconName}
					  file='main'
					  styles={styles}
					  imgParams={imgParams}
				/>
				<span className={styles["icon-our-work-text"]} {...microdata.ITEM_OFFERED_SERVICE}>
				  <span itemProp="name">
				    <Link href={url} passHref>
					  {children}
                    </Link>
				  </span>
				</span>
			</li>
		);
	};

	return (
		<section className={mk("container section about-company-main", styles)}>
			<div className={mk("container content-block about-company", styles)}>
				<h1 id={styles[site.data.nav.about.anchor_]}>О компании</h1>
				<div itemProp="description">
					<p>ИнноСтан оказывает услуги по&nbsp;ремонту, модернизации, поставке станков, разработке
						программного обеспечения и&nbsp;автоматизации линий производств.</p>

					<p>Наша команда &mdash; высококвалифицированные
						разработчики более&nbsp;30&nbsp;лет работающие в&nbsp;станкостроении.
					</p>
					<p>Наша работа &mdash; внедрение инноваций согласно Вашим целям.</p>
					<p>
						Мы предлагаем индивидуальные промышленные решения для&nbsp;стимулирования
						роста производства.
					</p>
				</div>
				<h2>Мы выполняем</h2>
				<ul className={styles['our-servs']} {...microdata.OFFER_CATALOG}>
					<LiItem liClass='service-li-repair' iconName='repair-middle' url={site.data.nav.s_repair.url}>
						<><a>ремонт металлообрабатывающих станков</a>;</>
					</LiItem>
					<LiItem liClass='service-li-shipping' iconName='shipping-start' url={site.data.nav.s_shipping.url}>
						<><a>поставку и&nbsp;пусконаладку металлообрабатывающих станков</a>;</>
					</LiItem>
					<LiItem liClass='li-modernization' iconName='robot' url={site.data.nav.s_modernization.url}>
						<><a>модернизацию металлообрабатывающих станков</a>;</>
					</LiItem>
					<LiItem liClass='service-li-controller' iconName='controller' url={site.data.nav.s_complex.url}>
						<><a>разработку и&nbsp;внедрение систем машинного зрения</a>;</>
					</LiItem>
					<LiItem liClass='service-li-intelligence' iconClass={'intelligence'} iconName='intelligence-ic' url={site.data.nav.s_complex.url}>
						<><a>разработку и&nbsp;внедрение систем управления с искусственным интеллектом</a>;</>
					</LiItem>
				</ul>
				<p>
					После внедрения продукта мы&nbsp;обеспечиваем его&nbsp;полную поддержку.
				</p>
				<h2>Специализация команды позволяет выполнять работы по</h2>
				<ul className={styles["our-spec"]} {...microdata.OFFER_CATALOG}>
					<li {...microdata.ITEM_LIST_OFFER}>
						<Icon icon="repair"
							  spanCover={styles["li-cover"]}
							  className={mk("icon-our-work s-repair", styles)}
							  file='main'
							  styles={styles}
							  imgParams={imgParams}
						/>
						<div className={styles["icon-our-work-text"]} {...microdata.ITEM_OFFERED_SERVICE}>
							<p className="descr" itemProp="name">
								<Link href={site.data.nav.s_repair.url} passHref><a>восстановлению, </a></Link>
								<Link href={site.data.nav.s_modernization.url} passHref><a>модернизации, </a></Link>
								<Link href={site.data.nav.s_repair.url} passHref><a>ремонту металлообрабатывающих станков</a></Link>,
							</p>
							<p className={mk("howlong", styles)}>
								более 30&nbsp;лет опыта&ensp;|&nbsp;&nbsp;
								<time dateTime="1988">1988</time>
								&nbsp;−&nbsp;
								<time dateTime={`${site.data.config.currentYear}`}>{site.data.config.currentYear}</time>
							</p>
						</div>
					</li>
					<li {...microdata.ITEM_LIST_OFFER}>
						<Icon icon="code"
							  spanCover={styles["li-cover"]}
							  className={mk("icon-our-work s-code", styles)}
							  file='main'
							  styles={styles}
							  imgParams={imgParams}
						/>
						<div className={styles["icon-our-work-text"]} {...microdata.ITEM_OFFERED_SERVICE }>
							<p className={styles["descr"]} itemProp="name">
								<Link href={ site.data.nav.s_complex.url } passHref>
									<a>разработке программного обеспечения для&nbsp;прикладных и&nbsp;встраиваемых систем</a>
								</Link>;
							</p>
							<p className={styles["howlong"]}>
								более 30&nbsp;лет опыта&ensp;|&nbsp;&nbsp;
								<time dateTime="1988">1988</time>
								&nbsp;−&nbsp;
								<time dateTime={`${site.data.config.currentYear}`}>{site.data.config.currentYear}</time>
							</p>
						</div>
					</li>
				</ul>
			</div>
			<noscript>
				<img itemProp="logo"
					 src={`${ site.domains.domain_canonical }stankostroitelnyj-zavod-innostan-logo-internal.png`}
					 {...imgParams['stankostroitelnyj-zavod-innostan-logo-internal.png']}
				/>
				<Link href={ site.domains.domain_canonical } passHref><a itemProp="url">Сайт ООО «ИнноСтан»</a></Link>
			</noscript>
		</section>
	)
}

export default AboutCompany;

