
String WWW_view_bool(uint8_t selected_value, String name_param, String arg_name) {
  String out = "<tr><td>" + name_param + ":</td><td><form method='post' action='/edit'><select name='" + arg_name + "' onchange='this.form.submit()'>";
  out = out + "<option value='0'"; if (selected_value == 0) out = out + " selected"; out = out + ">ВЫКЛ</option>";
  out = out + "<option value='1'"; if (selected_value != 0) out = out + " selected"; out = out + ">ВКЛ</option>";
  out = out + "</select></form></td></tr>\n";
  return out;
}

String WWW_view_bool_real(uint8_t selected_value, int real_value, String name_param, String arg_name, bool dis_off) {
  String out = "<tr><td>" + name_param + ":</td><td><form method='post' action='/edit'><select name='" + arg_name + "' ";
  if ((conditioner.ac_power == 0) and (dis_off)) out = out + "disabled ";
  out = out + "onchange='this.form.submit()'>";
  out = out + "<option value='0'"; if (selected_value == 0) out = out + " selected"; out = out + ">ВЫКЛ</option>";
  out = out + "<option value='1'"; if (selected_value != 0) out = out + " selected"; out = out + ">ВКЛ</option>";
  out = out + "</select></form>реально: ";

  if      (real_value == 0) { out = out + "ВЫКЛ"; }
  else if (real_value == 1) { out = out + "ВКЛ";  }
  else                      { out = out + real_value; }

  out = out + "</td></tr>\n";
  return out;
}

String WWW_view_target_temperature(int selected_value, int real_value) {
  String out = "<tr><td>Поддерживаемая температура:</td><td><form method='post' action='/edit'><select name='target_temperature' ";
  if (conditioner.ac_power == 0) out = out + "disabled ";
  out = out + " onchange='this.form.submit()'>";
  for (int i = 16; i <= 31; i++) {
    out = out + "<option value='" + String(i) + "'";
    if (selected_value == i) out = out + " selected";
    out = out + ">" + String(i) + "</option>";
  }
  out = out + "</select></form>реально: " + real_value + "</td></tr>\n";
  return out;
}

String WWW_view_ac_mode(int selected_value, int real_value) {
  String out = "<tr><td>Режим работы:</td><td><form method='post' action='/edit'> <select name='ac_mode'  ";
  if (conditioner.ac_power == 0) out = out + "disabled ";
  out = out + " onchange='this.form.submit()'>";
  out = out + "<option value='1'"; if (selected_value == 1) out = out + " selected"; out = out + ">1. Автоматический</option>";
  out = out + "<option value='2'"; if (selected_value == 2) out = out + " selected"; out = out + ">2. Охлаждение</option>";
  out = out + "<option value='3'"; if (selected_value == 3) out = out + " selected"; out = out + ">3. Нагрев</option>";
  out = out + "<option value='4'"; if (selected_value == 4) out = out + " selected"; out = out + ">4. Только вентилятор</option>";
  out = out + "<option value='5'"; if (selected_value == 5) out = out + " selected"; out = out + ">5. Осушение</option>";
  out = out + "</select></form>реально: ";

  if (real_value == 0) out = out + "ВЫКЛ";
  if (real_value == 1) out = out + "Автоматический";
  if (real_value == 2) out = out + "Охлаждение";
  if (real_value == 3) out = out + "Нагрев";
  if (real_value == 4) out = out + "Только вентилятор";
  if (real_value == 5) out = out + "Осушение";

  out = out + "</td></tr>\n";
  return out;
}

String WWW_view_fan_mode(int selected_value, int real_value) {
  String out = "<tr><td>Вентилятор:</td><td><form method='post' action='/edit'><select name='fan_mode' ";
  if (conditioner.ac_power == 0) out = out + "disabled ";
  out = out + "onchange='this.form.submit()'>";
  out = out + "<option value='1'"; if (selected_value == 1) out = out + " selected"; out = out + ">1. ВЫКЛ</option>";
  out = out + "<option value='2'"; if (selected_value == 2) out = out + " selected"; out = out + ">2. Автоматический</option>";
  out = out + "<option value='3'"; if (selected_value == 3) out = out + " selected"; out = out + ">3. Минимум</option>";
  out = out + "<option value='4'"; if (selected_value == 4) out = out + " selected"; out = out + ">4. Средний</option>";
  out = out + "<option value='5'"; if (selected_value == 5) out = out + " selected"; out = out + ">5. Выше среднего</option>";
  out = out + "<option value='6'"; if (selected_value == 6) out = out + " selected"; out = out + ">6. Максимальный</option>";
  out = out + "<option value='7'"; if (selected_value == 7) out = out + " selected"; out = out + ">7. Режим \"Focus\"</option>";
  out = out + "<option value='8'"; if (selected_value == 8) out = out + " selected"; out = out + ">8. Режим \"Diffuse\"</option>";
  out = out + "<option value='9'"; if (selected_value == 9) out = out + " selected"; out = out + ">9. Режим \"Quiet\"</option>";
  out = out + "</select></form>реально: ";

  if (real_value == 1) out = out + "ВЫКЛ";
  if (real_value == 2) out = out + "Автоматический";
  if (real_value == 3) out = out + "Минимум";
  if (real_value == 4) out = out + "Средний";
  if (real_value == 5) out = out + "Выше среднего";
  if (real_value == 6) out = out + "Максимальный";
  if (real_value == 7) out = out + "Режим \"Focus\"";
  if (real_value == 8) out = out + "Режим \"Diffuse\"";
  if (real_value == 9) out = out + "Режим \"Quiet\"";

  out = out + "</td></tr>\n";
  return out;
}

String WWW_view_swing_mode(int selected_value, int real_value) {
  String out = "<tr><td>Режим качения шторок:</td><td><form method='post' action='/edit'><select name='swing_mode' ";
  if (conditioner.ac_power == 0) out = out + "disabled ";
  out = out + "onchange='this.form.submit()'>";
  out = out + "<option value='0'"; if (selected_value == 0) out = out + " selected"; out = out + ">0. ВЫКЛ</option>";
  out = out + "<option value='1'"; if (selected_value == 1) out = out + " selected"; out = out + ">1. Качение в обоих плоскостях</option>";
  out = out + "<option value='2'"; if (selected_value == 2) out = out + " selected"; out = out + ">2. Вертикальное качение</option>";
  out = out + "<option value='3'"; if (selected_value == 3) out = out + " selected"; out = out + ">3. Горизонтальное качение</option>";
  out = out + "</select></form>реально: ";

  if (real_value == 0) out = out + "ВЫКЛ";
  if (real_value == 1) out = out + "Качение в обоих плоскостях";
  if (real_value == 2) out = out + "Вертикальное качение";
  if (real_value == 3) out = out + "Горизонтальное качение";

  out = out + "</td></tr>\n";
  return out;
}

String WWW_v_swing_mode(int selected_value) {
  String out = "<tr><td>Качание вертикальной заслонки:</td><td><form method='post' action='/edit'> <select name='v_swing_mode' onchange='this.form.submit()'>";
  out = out + "<option value='0'"; if (selected_value == 0) out = out + " selected"; out = out + ">0. Вверх-вниз</option>";
  out = out + "<option value='1'"; if (selected_value == 1) out = out + " selected"; out = out + ">1. Сверху</option>";
  out = out + "<option value='2'"; if (selected_value == 2) out = out + " selected"; out = out + ">2. Снизу</option>";
  out = out + "</select></form></td></tr>\n";
  return out;
}

String WWW_h_swing_mode(int selected_value) {
  String out = "<tr><td>Качание горизонтальных заслонок:</td><td><form method='post' action='/edit'> <select name='h_swing_mode' onchange='this.form.submit()'>";
  out = out + "<option value='0'"; if (selected_value == 0) out = out + " selected"; out = out + ">0. Лево-право</option>";
  out = out + "<option value='1'"; if (selected_value == 1) out = out + " selected"; out = out + ">1. Слева</option>";
  out = out + "<option value='2'"; if (selected_value == 2) out = out + " selected"; out = out + ">2. В центре</option>";
  out = out + "<option value='3'"; if (selected_value == 3) out = out + " selected"; out = out + ">3. Справа</option>";
  out = out + "</select></form></td></tr>\n";
  return out;
}
String WWW_v_fixing_mode(int selected_value) {
  String out = "<tr><td>Фиксация вертикальной заслонки:</td><td><form method='post' action='/edit'> <select name='v_fixing_mode' onchange='this.form.submit()'>";
  out = out + "<option value='0'"; if (selected_value == 0) out = out + " selected"; out = out + ">0. Установленная позиция</option>";
  out = out + "<option value='1'"; if (selected_value == 1) out = out + " selected"; out = out + ">1. Верх</option>";
  out = out + "<option value='2'"; if (selected_value == 2) out = out + " selected"; out = out + ">2. Выше середины</option>";
  out = out + "<option value='3'"; if (selected_value == 3) out = out + " selected"; out = out + ">3. Середина</option>";
  out = out + "<option value='4'"; if (selected_value == 4) out = out + " selected"; out = out + ">4. Ниже середины</option>";
  out = out + "<option value='5'"; if (selected_value == 5) out = out + " selected"; out = out + ">5. Низ</option>";
  out = out + "</select></form></td></tr>\n";
  return out;
}
String WWW_h_fixing_mode(int selected_value) {
  String out = "<tr><td>Фиксация горизонтальных заслонок:</td><td><form method='post' action='/edit'> <select name='h_fixing_mode' onchange='this.form.submit()'>";
  out = out + "<option value='0'"; if (selected_value == 0) out = out + " selected"; out = out + ">0. Установленная позиция</option>";
  out = out + "<option value='1'"; if (selected_value == 1) out = out + " selected"; out = out + ">1. Лево</option>";
  out = out + "<option value='2'"; if (selected_value == 2) out = out + " selected"; out = out + ">2. Левее середины</option>";
  out = out + "<option value='3'"; if (selected_value == 3) out = out + " selected"; out = out + ">3. Середина</option>";
  out = out + "<option value='4'"; if (selected_value == 4) out = out + " selected"; out = out + ">4. Правее середины</option>";
  out = out + "<option value='5'"; if (selected_value == 5) out = out + " selected"; out = out + ">5. Право</option>";
  out = out + "</select></form></td></tr>\n";
  return out;
}
