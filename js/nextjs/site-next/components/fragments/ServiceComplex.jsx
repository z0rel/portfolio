import React from "react";
import Icon from "../Svg/Icon";
import microdata from "../microdata/microdata";
import site from "../constants/siteConstants";
import {mk} from "../utils/mk";
import Link from "next/link";


const ServiceComplex = ({styles, imgParams, notitle}) => {
	return <li className={styles["service-custom"]} {...microdata.ITEM_LIST_OFFER}>
        <Link href={ site.data.nav.s_complex.url } passHref>
			<a {...microdata.ITEM_OFFERED_SERVICE}>
				<div className={mk("subblock service-complex-img", styles)}>
					<div className={mk("service-row", styles)}>
						<div className={mk("service-col1 s-block-intelligence", styles)}>
							<Icon className={styles["s-intelligence"]} icon="intelligence-img" file='main' imgParams={imgParams} styles={styles}/>
							<div className={styles['serv-text']}>Машинное обучение</div>
						</div>
						<div className={mk("service-col1 s-block-computer-vision", styles)}>
							<Icon className={styles["s-computer-vision"]} icon="computer-vision" file='main' imgParams={imgParams} styles={styles}/>
							<div className={styles['serv-text']}>Компьютерное зрение</div>
						</div>
						<div className={mk("service-col1 s-block-networks", styles)}>
							<Icon className={styles["s-networks"]} icon="networks" file='main' imgParams={imgParams} styles={styles}/>
							<div className={styles['serv-text']}>Сетевая интеграция</div>
						</div>
					</div>
					<div className={styles["service-row2"]}>
						<div className={mk("service-col2 s-block-automation", styles)}>
							<Icon className={styles["s-automation"]} icon="automation-img" file='main' imgParams={imgParams} styles={styles}/>
							<div className={styles['serv-text']}>Автоматизация</div>
						</div>
						<div className={mk("service-col2 s-block-applications", styles)}>
							<Icon className={styles["s-applications"]} icon="applications" file='main' imgParams={imgParams} styles={styles}/>
							<div className={styles['serv-text']}>Удаленная диагностика и&nbsp;управление</div>
						</div>
					</div>
				</div>
				{!notitle ? <div className={styles["subblock"]} itemProp="name" dangerouslySetInnerHTML={{__html: site.data.nav.s_complex.title}}></div> : <></>}
			</a>
		</Link>
	</li>
}
export default ServiceComplex;