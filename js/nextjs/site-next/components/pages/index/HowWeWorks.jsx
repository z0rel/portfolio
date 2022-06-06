import {mk} from "../../utils/mk";

const HowWeWorks = ({styles}) => {

	let TimelineItem = ({timelineClass, title, children}) => {
		return <li className={mk(`timeline-item ${timelineClass}`, styles)}
			itemProp="step"
			itemScope
			itemType="http://schema.org/HowToStep">
			<div className={styles["caps"]} itemProp="name">{title}</div>
			<p className={styles["descr"]} itemProp="description">
				{children}
			</p>
		</li>
	}

	return (
		<section className={mk("container-fluid how-we-works-main", styles)}>
			<div className={styles["container"]}>
				<div className={styles["row"]}>
					<div className={styles["content-block"]}>
						<div className={styles["how-we-works"]} itemScope itemType="http://schema.org/HowTo">
							<h1 itemProp="name">Как мы работаем</h1>
							<ol className={styles["timeline-block"]}>
								<TimelineItem timelineClass='requirements' title='Требования'>
									Анализ Ваших производственных потребностей и&nbsp;сбор
									требований к&nbsp;проекту<span className={mk("d-inline d-sm-none", styles)}>.</span>
								</TimelineItem>
								<TimelineItem timelineClass='estimate' title='Оценка'>
									Оценка ресурсов и&nbsp;стоимости проекта<span className={mk("d-inline d-sm-none", styles)}>.</span>
								</TimelineItem>
								<TimelineItem timelineClass='offer' title='Предложение'>
									Разработка, обсчет и&nbsp;согласование технико-коммерческого предложения<span className={mk("d-inline d-sm-none", styles)}>.</span>
								</TimelineItem>
								<TimelineItem timelineClass='contract' title='Договор'>
									Согласование договора.
									<span className={mk("d-none d-sm-inline ", styles)}><br/></span>Подписание
									<span className={mk("d-none d-sm-inline ", styles)}><br/></span>договора<span
									className={mk("d-inline d-sm-none", styles)}>.</span>
								</TimelineItem>
								<TimelineItem timelineClass='implementing' title='Выполнение'>
									Выполнение работ согласно договору<span className={mk("d-inline d-sm-none", styles)}>.</span>
								</TimelineItem>
								<TimelineItem timelineClass='control' title='Сдача'>
									Приёмо-сдаточные испытания. <span className={mk("d-inline d-sm-none", styles)}><br/></span>
									Обучение специалистов Заказчика<span className={mk("d-inline d-sm-none", styles)}>.</span>
								</TimelineItem>
								<TimelineItem timelineClass='support' title='Сопровождение'>
									Гарантийная поддержка и&nbsp;сопровождение<span className={mk("d-inline d-sm-none", styles)}>.</span>
								</TimelineItem>
							</ol>
						</div>
					</div>
				</div>
			</div>
		</section>
	);
}
export default HowWeWorks;