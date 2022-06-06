import ServiceIcon from "../../fragments/ServiceIcon";
import site from "../../constants/siteConstants";
import ServiceComplex from "../../fragments/ServiceComplex";
import microdata from "../../microdata/microdata";
import {mk} from "../../utils/mk";

const ServicesSection = ({styles, imgParams}) => {
	return <section className={mk("container services-section-main", styles)} id={styles[site.data.nav.service_main.anchor_]}>
		<h1 style={{display: "none"}}>Услуги</h1>
		<div className={styles["row"]}>
			<div className={mk("col-12 slogan-2", styles)}>
				<p itemProp="description">
					<span className={styles["no-break-nobr"]}>ООО &laquo;ИнноСтан&raquo;</span>
					&#8722; от&nbsp;ремонта станков и&nbsp;промышленного оборудования до&nbsp;создания производственных линий для Ваших бизнес&#8209;задач.
				</p>
			</div>
		</div>
		<div className={mk("row services-section", styles)}>
			<ul className={styles["col-12"]} {...microdata.OFFER_CATALOG}>
				<ServiceIcon incl_data={site.data.nav.s_repair} imgParams={imgParams} styles={styles}/>
				<ServiceIcon incl_data={site.data.nav.s_modernization} imgParams={imgParams} styles={styles}/>
				<ServiceIcon incl_data={site.data.nav.s_shipping} imgParams={imgParams} styles={styles}/>
				<ServiceComplex imgParams={imgParams} styles={styles} />
				<ServiceIcon incl_data={site.data.nav.s_integration} imgParams={imgParams} styles={styles}/>
				<li className={styles["service-custom"]} />
			</ul>
		</div>
	</section>
}

export default ServicesSection;