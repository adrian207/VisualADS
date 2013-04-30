package cnd.conflict.util;

import java.awt.*;
import javax.swing.border.*;
import java.net.*;
import javax.swing.*;

import cnd.conflict.detect.Report;
import cnd.conflict.frame.Frame;

import java.awt.event.*;


/**
 * �������öԻ������
 */
public class Help extends JFrame {

	Frame frame;
	JPanel titlePanel = new JPanel();
	JPanel contentPanel = new JPanel();
	JPanel closePanel = new JPanel();

	JButton close = new JButton();
	JLabel title = new JLabel("���߰���");
	JTextArea help = new JTextArea(); 

	Color bg = new Color(255,255,255);

	public Help(Frame frame) {
		try {
			this.frame = frame;
			jbInit();
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		//��������λ�ã�ʹ�Ի������
		Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
		this.setLocation( (int) (screenSize.width - 400) / 2,
						(int) (screenSize.height - 320) / 2);
		this.setResizable(false);
		this.setVisible(true);
	}

	public void jbInit() throws Exception {
		
		this.setSize(new Dimension(400, 200));
		if (frame == null || frame.getLanguage() == 1) {
			this.setTitle("����");
			title.setText("���߰���");
		}
		else {
			this.setTitle("Help");
			title.setText("Tool Help");
		}
		
		titlePanel.setBackground(bg);;
		contentPanel.setBackground(bg);
		closePanel.setBackground(bg);
		
		if (frame == null || frame.getLanguage() == 1)
			help.setText("���������������Գ�ͻ��⹤��\n");
		else
			help.setText("Computer Network Defence Policy Conflict Detecting Tool\n");
		help.setEditable(false);

		titlePanel.add(new Label("              "));
		titlePanel.add(title);
		titlePanel.add(new Label("              "));

		contentPanel.add(help);

		closePanel.add(new Label("              "));
		closePanel.add(close);
		closePanel.add(new Label("              "));

		Container contentPane = getContentPane();
		contentPane.setLayout(new BorderLayout());
		contentPane.add(titlePanel, BorderLayout.NORTH);
		contentPane.add(contentPanel, BorderLayout.CENTER);
		contentPane.add(closePanel, BorderLayout.SOUTH);

		if (frame == null || frame.getLanguage() == 1)
			close.setText("�ر�");
		else
			close.setText("Close");
		//�¼�����
		close.addActionListener(
			new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					dispose();
				}
			}
		);
	}
	
	
	
	public static void main(String[] args){
		new Help(null);
	}
}