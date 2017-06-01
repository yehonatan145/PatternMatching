package com.pattern_matching;


import javax.swing.*;
import javax.swing.border.EmptyBorder;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableCellRenderer;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

public class MainForm {
    private JPanel panelMain;
    private JList dictionaryList;
    private JList streamList;
    private JButton btnTest;

    private DefaultListModel<JCheckBox> dictionaryModel = new DefaultListModel<>();
    private DefaultListModel<JCheckBox> streamModel = new DefaultListModel<>();

    public MainForm() {
        initCheckBoxList(dictionaryList, dictionaryModel);
        initCheckBoxList(streamList, streamModel);

        dictionaryModel.addElement(new JCheckBox("snort"));
        dictionaryModel.addElement(new JCheckBox("clamAV"));
        dictionaryModel.addElement(new JCheckBox("pulled-pork"));

        btnTest.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent actionEvent) {
                boolean isFirst = true;
                String res = "";
                for (int i = 0; i < dictionaryModel.getSize(); i++) {
                    JCheckBox checkbox = dictionaryModel.getElementAt(i);
                    if (checkbox.isSelected()) {
                        res += (isFirst ? "" : ", ") + checkbox.getText();
                        isFirst = false;
                    }
                }
                btnTest.setText(res);
            }
        });
    }

    public static void main(String[] args) {
        JFrame frame = new JFrame();
        frame.setContentPane(new MainForm().panelMain);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.pack();
        frame.setVisible(true);
    }

    /**
     * initalize the {@link JList<JCheckBox>} to the settings we want
     *
     * @param list the list of JCheckBox
     */
    public void initCheckBoxList(JList<JCheckBox> list, DefaultListModel<JCheckBox> model) {
        list.setModel(model);
        model.addElement(new JCheckBox("all"));

        list.setCellRenderer(new ListCellRenderer<JCheckBox>() {
            @Override
            public Component getListCellRendererComponent(JList<? extends JCheckBox> jList, JCheckBox jCheckBox, int i, boolean isSelected, boolean b1) {

                //Drawing checkbox, change the appearance here
                jCheckBox.setBackground(isSelected ? jList.getSelectionBackground()
                        : jList.getBackground());
                jCheckBox.setForeground(isSelected ? jList.getSelectionForeground()
                        : jList.getForeground());
                jCheckBox.setEnabled(jList.isEnabled());
                jCheckBox.setFont(jList.getFont());
                jCheckBox.setFocusPainted(false);
                jCheckBox.setBorderPainted(true);
                jCheckBox.setBorder(isSelected ? UIManager
                        .getBorder("List.focusCellHighlightBorder") : new EmptyBorder(1, 1, 1, 1));
                return jCheckBox;
            }
        });
        list.addMouseListener(new MouseAdapter() {
            public void mousePressed(MouseEvent e) {
                int index = list.locationToIndex(e.getPoint());
                if (index != -1) {
                    if (index == 0) {
                        JCheckBox firstCheckbox = (JCheckBox)list.getModel().getElementAt(0);
                        boolean isSelected = firstCheckbox.isSelected();
                        firstCheckbox.setSelected(!isSelected);
                        for (int i = 1; i < list.getModel().getSize(); i++) {
                            JCheckBox checkbox = (JCheckBox) list.getModel().getElementAt(i);
                            checkbox.setSelected(!isSelected);
                        }
                    } else {
                        JCheckBox checkbox = (JCheckBox) list.getModel().getElementAt(index);
                        checkbox.setSelected(!checkbox.isSelected());
                    }
                    list.repaint();
                }
            }
        });
        list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
    }
}


