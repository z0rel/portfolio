import {mk} from "../../utils/mk";
import site from "../../constants/siteConstants";
import Icon from "../../Svg/Icon";
import React from "react";

const ContactsForm = ({styles, imgParams}) => {
    return (
        <>
        <section className={mk("container section contacts-form", styles)}>
            <h1 id={styles[site.data.nav.write_us.anchor_]}>Связаться с&nbsp;нами</h1>
            <p className={styles["please-input"]}>Пожалуйста, введите Ваши данные. В&nbsp;основной текстовой области Вы можете подробно изложить свой вопрос.</p>
            <p className={styles["our-spec"]}>Наши специалисты ответят Вам в&nbsp;кратчайшие сроки. Спасибо!</p>
            <form action={ `${site.domains.domain_backend}do/send.php` }
                  method="post"
                  className={styles["needs-validation"]}
                  id='form-back-mail'
                  noValidate
            >
                <div className={mk("input-group mb-3 name-area", styles)}>
                    <div className={styles["input-group-prepend"]}>
                        <span className={styles["input-group-text"]} id="basic-addon1">
                            <Icon className={styles["ic-person"]}
                                  icon="person"
                                  file='main'
                                  styles={styles}
                                  imgParams={imgParams}
                            />
                        </span>
                    </div>
                    <input id="form-name"
                           name="send-name"
                           type="text"
                           className={styles["form-control"]}
                           placeholder="Ваше имя"
                           aria-label="Ваше имя"
                           aria-describedby="basic-addon1"
                    />
                    <div className={styles["invalid-feedback"]}>Пожалуйста, введите Ваше имя.</div>
                </div>
                <div className={mk("input-group mb-3 email-area", styles)}>
                    <div className={styles["input-group-prepend"]}>
                        <span className={styles["input-group-text"]} id="basic-addon2">
                            <Icon className={styles["ic-mail"]}
                                  icon="mail"
                                  file='main'
                                  styles={styles}
                                  imgParams={imgParams}
                            />
                        </span>
                    </div>
                    <input id="form-email"
                           name="send-email"
                           type="text"
                           className={styles["form-control"]}
                           placeholder="Ваш e-mail адрес"
                           aria-label="Ваш e-mail адрес"
                           aria-describedby="basic-addon1"
                           type='email'
                    />
                    <div className={styles["invalid-feedback"]}>Пожалуйста, введите корректный e-mail.</div>
                </div>
                <div>
                    <div className={mk("input-group message-area", styles)}>
                        <div className={styles["input-group-prepend"]}>
                            <span className={styles["input-group-text"]}>
                                <Icon className={styles["ic-edit"]}
                                      icon="edit"
                                      file='main'
                                      styles={styles}
                                      imgParams={imgParams}
                                />
                            </span>
                        </div>
                        <div className={styles["wrap"]}>
                            <div className={styles["pull-tab"]}></div>
                            <textarea id="form-message"
                                      name="send-message"
                                      className={styles["form-control"]}
                                      rows={9}
                                      cols={40}
                                      placeholder="Ваше сообщение"
                                      aria-label="Ваше сообщение"
                            />
                    </div>
                </div>
                <div className={mk("invalid-feedback inv-message", styles)}>Пожалуйста, введите Ваше сообщение.</div>
            </div>
            <div className={styles["btn-wrap"]} itemScope itemType="http://schema.org/CommunicateAction">
                <button id="form-button"
                        className={mk("btn btn-primary btn-lg", styles)}
                        role="button"
                >
                    Отправить сообщение
                </button>
            </div>
            <div className={mk("invalid-feedback val-frm i1", styles)}>
                Извините. К&nbsp;сожалению Ваше сообщение не&nbsp;удалось
                отправить <span className={styles["no-break-nobr"]}>из-за</span> того, что
                на&nbsp;нашем сервере возникла ошибка.
            </div>
            <div className={mk("invalid-feedback val-frm", styles)}>
                Мы&nbsp;уже работаем над ее&nbsp;устранением. Пожалуйста, повторите запрос позже.
            </div>
            <div className={mk("valid-feedback val-frm", styles)}>
                Ваше сообщение принято. Спасибо, что обращаетесь к нам.
            </div>
        </form>
        </section>
     </>
    );
}

export default ContactsForm;