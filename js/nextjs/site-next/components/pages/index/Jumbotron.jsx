import Link from "next/link";
import site from "../../constants/siteConstants";
import {mk} from "../../utils/mk";

const Jumbotron = ({styles}) => {
	return (
		<div className={mk("row jumbotron-row", styles)}>
			<div className={mk("col-12 jumbotron-col", styles)}>
				<div className={mk("jumbotron jumbotron-fluid", styles)}>
					<div className={mk("display-4 header-title", styles)}>
						Разработка промышленных решений будущего&nbsp;сегодня
					</div>
					<div className={styles.row}>
						<div className={styles["col-12"]}>
							<p className={mk("lead header-title-2", styles)}>
								Команда
								<span itemProp="name">ИнноСтан</span> — инновации в&nbsp;станкостроении
								и&nbsp;интеллектуальных&nbsp;системах
							</p>
						</div>
					</div>
					<p className={styles["head-button-p"]} itemScope itemType="http://schema.org/CommunicateAction">
						<Link href={`#${ site.data.nav.write_us.anchor_}`} scroll={false} passHref>
							<a id="jumbotronBtn" className={mk("btn btn-primary btn-lg", styles)} role="button">
								НАПИСАТЬ НАМ
							</a>
						</Link>
					</p>
				</div>
			</div>
		</div>
	);
}

export default Jumbotron