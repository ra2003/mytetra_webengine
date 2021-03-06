
#if QT_VERSION == 0x050600
#include <wobjectimpl.h>
#endif

#include <QBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QStringList>
#include <QWidget>

#include "app_config_page_table.h"
#include "controllers/record_table/record_controller.h"
#include "libraries/disk_helper.h"
#include "libraries/fixed_parameters.h"
#include "libraries/global_parameters.h"
#include "main.h"
#include "models/app_config/app_config.h"
#include "models/record_table/record_index.hxx"
#include "models/record_table/record_model.h"
#include "views/browser/browser.h"
#include "views/browser/tabwidget.h"
#include "views/main_window/hidable_tab.h"
#include "views/record_table/record_screen.h"

extern std::shared_ptr<AppConfig> appconfig;
extern FixedParameters fixedparameters;
extern std::shared_ptr<gl_para> gl_paras;

#if QT_VERSION == 0x050600
W_OBJECT_IMPL(AppConfigPageTable)
#endif

AppConfigPageTable::AppConfigPageTable(
    QWidget* parent) // rctrl_t *record_controller,
    : ConfigPage(parent)
{
	//	  , _record_controller(record_controller)
	qDebug() << "Create record table config page";

	QStringList all_fields_key_name = fixedparameters._record_all_fields_key_list;
	// all_field_names << "close";
	QMap<QString, QString> description_fields = fixedparameters.record_field_description(all_fields_key_name);

	// description_fields["close"] = tr("Close item");
	QStringList showFields = appconfig->record_table_show_fields();
	// Создаются чекбоксы для каждого поля, хранимого в записи
	for (int i = 0; i < all_fields_key_name.size(); i++) {
		QString name = all_fields_key_name[i];
		_fields[name] = new QCheckBox(this);

		_fields[name]->setText(tr(description_fields.value(name).toLocal8Bit().data()));
		if (showFields.contains(name)) _fields[name]->setCheckState(Qt::Checked);
	}
	// Область настройки видимости заголовков
	_show_horizontal_header = new QCheckBox(this);
	_show_horizontal_header->setText(tr("Show horisontal header"));
	if (appconfig->record_table_show_horizontal_headers())
		_show_horizontal_header->setCheckState(Qt::Checked);
	_show_vertical_header = new QCheckBox(this);
	_show_vertical_header->setText(tr("Show row number"));
	if (appconfig->record_table_show_vertical_headers())
		_show_vertical_header->setCheckState(Qt::Checked);
	QVBoxLayout* vboxVisibleHeaders = new QVBoxLayout;
	vboxVisibleHeaders->addWidget(_show_horizontal_header);
	vboxVisibleHeaders->addWidget(_show_vertical_header);

	QGroupBox* groupBoxVisibleHeaders = new QGroupBox(tr("Headers and numbers visible"));
	groupBoxVisibleHeaders->setLayout(vboxVisibleHeaders);

	// Область настройки видимости столбцов
	QVBoxLayout* vboxVisibleColumns = new QVBoxLayout;

	foreach (QCheckBox* currentCheckBox, _fields)
		vboxVisibleColumns->addWidget(currentCheckBox);

	QGroupBox* groupBoxVisibleColumns = new QGroupBox(tr("Columns visible"));
	groupBoxVisibleColumns->setLayout(vboxVisibleColumns);

	// Собирается основной слой
	QVBoxLayout* central_layout = new QVBoxLayout();

	central_layout->addWidget(groupBoxVisibleHeaders);
	central_layout->addWidget(groupBoxVisibleColumns);
	central_layout->addStretch();

	// Устанавливается основной слой
	setLayout(central_layout);

	setupSignals();
}

void AppConfigPageTable::setupSignals(void)
{
	QMapIterator<QString, QCheckBox*> i(_fields);
	while (i.hasNext()) {
		i.next();
		connect(i.value(), &QCheckBox::toggled, this, &AppConfigPageTable::on_field_toggle);
	}
	//	// Указатель на контроллер таблицы конечных записей
	//	// RecordController *_record_controller =
	//find_object<RecordController>("table_screen_controller");
	//	// При изменении настроек отображения таблицы конечных записей должен
	//вызываться соответствующий слот контроллера (чтобы перечиталась ширина
	//столбцов)
	//	if(_record_controller) connect(this,
	//&AppConfigPageTable::record_table_config_change, _record_controller,
	//&rctrl_t::on_recordtable_configchange);
	//// connect(this, &AppConfigPageTable::recordTableConfigChange,
	//page_controller, &RecordController::on_recordtable_configchange);
	auto _vtab_record = gl_paras->vtab_record();
	int count_browser = 0;
	for (int i = 0; i < _vtab_record->count(); i++) {
		auto rs = _vtab_record->widget(i);
		if (rs->objectName() == record_screen_multi_instance_name) {
			// auto	rs		= dynamic_cast<rs_t *>(w);
			auto bro_ = dynamic_cast<rs_t*>(rs)->browser();
			if (bro_) {
				auto _rctrl = bro_->tab_widget()->record_screen()->record_ctrl();
				if (_rctrl)
					connect(this, &AppConfigPageTable::record_table_config_change, _rctrl, &rctrl_t::on_recordtable_configchange);
			}
			count_browser++;
		}
	}
}

// Слот, срабатывающий каждый раз когда изменяется чекбокс любого поля
void AppConfigPageTable::on_field_toggle(bool flag)
{
	Q_UNUSED(flag);

	int count = 0;

	QMapIterator<QString, QCheckBox*> i(_fields);
	while (i.hasNext()) {
		i.next();
		if (i.value()->isChecked())
			count++;
	}
	// Если пользователь снял флажки со всех полей
	if (count == 0) {
		QMessageBox msgBox;
		msgBox.setText("Can't check off all fields.");
		msgBox.exec();

		_fields["name"]->setCheckState(Qt::Checked);
	}
}

// Метод должен возвращать уровень сложности сделанных изменений
// 0 - изменения не требуют перезапуска программы
// 1 - изменения требуют перезапуска программы
int AppConfigPageTable::apply_changes(void)
{
	qDebug() << "Apply changes record table";

	// Запоминается ширина полей
	// Это надо сделать в первую очередь, потому что в дальнейшем после
	// перечитывания модели и
	// установки заголовков таблицы конечных записей слетают ширины полей
	// (устанавливаюся в 100 px самим Qt)
	QStringList show_fields = appconfig->record_table_show_fields();
	QStringList fields_width = appconfig->record_table_fields_width();
	qDebug() << "showFields" << show_fields;
	qDebug() << "fieldsWidth" << fields_width;
	// Запоминание в конфигурацию отображения горизонтальных заголовков
	if (appconfig->record_table_show_horizontal_headers() != _show_horizontal_header->isChecked())
		appconfig->record_table_show_horizontal_headers(_show_horizontal_header->isChecked());
	// Запоминание в конфигурацию отображения нумерации строк
	if (appconfig->record_table_show_vertical_headers() != _show_vertical_header->isChecked())
		appconfig->record_table_show_vertical_headers(_show_vertical_header->isChecked());
	QStringList add_fields_list;    // Список полей, которые добавились в результате
					// настройки
	QStringList remove_fields_list; // Список полей, которые должны удалиться в
					// результате настройки

	// Определение, какие поля нужно добавить, какие удалить
	QMapIterator<QString, QCheckBox*> i(_fields);
	while (i.hasNext()) {
		i.next();
		// Если поле добавилось
		if (!show_fields.contains(i.key()) && i.value()->isChecked()) add_fields_list << i.key();
		// Если поле удалилось
		if (show_fields.contains(i.key()) && !i.value()->isChecked()) remove_fields_list << i.key();
	}
	// if(appconfig->record_table_show_vertical_headers() && !
	// add_fields_list.contains("row number"))add_fields_list << "row number";
	qDebug() << "add_fields_list : " << add_fields_list;
	qDebug() << "remove_fields_list : " << remove_fields_list;

	// езультирующий список полей
	QStringList new_show_fields;

	// Добавление полей в результирующий список
	new_show_fields = show_fields + add_fields_list;

	// Удаление полей в результирующем списке
	foreach (QString remove_field_name, remove_fields_list)
		new_show_fields.removeAll(remove_field_name);

	qDebug() << "new_show_fields : " << new_show_fields;

	// Установка имен полей в конфигурацию
	appconfig->record_table_show_fields(new_show_fields);
	// Если полей становится больше чем было
	if (new_show_fields.size() > show_fields.size()) {
		qDebug() << "Add fields width process...";

		// Ширина всех полей
		float full_width = 0.0;

		foreach (QString current_width, fields_width)
			full_width += current_width.toFloat();

		qDebug() << "fullWidth" << full_width;

		// Уменьшается ширина существующих полей
		QStringList new_fields_width;
		float insert_field_width = 100.0;
		float insert_full_width =
		    insert_field_width * (new_show_fields.size() - show_fields.size());
		float coefficient = (full_width - insert_full_width) / full_width;

		foreach (QString current_width, fields_width)
			new_fields_width << QString::number((int)(current_width.toFloat() * coefficient));

		qDebug() << "insertFullWidth" << insert_full_width;
		qDebug() << "coefficient" << coefficient;
		qDebug() << "newFieldsWidth" << new_fields_width;
		// Добавляются ширины добавленных полей
		for (int n = 0; n < (new_show_fields.size() - show_fields.size()); n++) new_fields_width << QString::number((int)insert_field_width);
		qDebug() << "new_fields_width" << new_fields_width;

		// Новые ширины полей запомниаются в конфигурацию
		appconfig->record_table_fields_width(new_fields_width);
	}
	// Если полей становится меньше чем было
	if (new_show_fields.size() < show_fields.size()) {
		qDebug() << "Remove fields width process...";

		QStringList new_fields_width = fields_width;

		qDebug() << "newFieldsWidth" << new_fields_width;
		for (int n = 0; n < (new_show_fields.size() - show_fields.size()); n++) new_fields_width.removeLast();
		qDebug() << "newFieldsWidth in result" << new_fields_width;

		// Установка новых ширин полей в конфигурацию
		appconfig->record_table_fields_width(new_fields_width);
	}
	// Если полей столько же сколько и было
	if (new_show_fields.size() == show_fields.size()) {
		qDebug() << "Count of field not changed. Set previous fields width"
			 << fields_width;

		// Установка запомненных ширин полей в конфигурацию
		// Так как это значение в конфигурации было искажено в момент
		// переподключения модели
		appconfig->record_table_fields_width(fields_width);
	}
	emit record_table_config_change();

	return 0;
}
