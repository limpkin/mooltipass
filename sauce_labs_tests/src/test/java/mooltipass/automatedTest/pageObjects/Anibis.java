package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Anibis extends AbstractPage{

	public Anibis(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}
	
	@FindBy(id = "ctl00_phlContent_txtUsername")
	private WebElement email;

	@FindBy(id = "ctl00_phlContent_txtPassword")
	private WebElement password;

	@FindBy(id = "ctl00_Header1_ctlHeaderNavBar_ctlHeaderLoginStateControlls_hypLoginRegister")
	private WebElement loginBtn;
	
	@FindBy(id = "ctl00_phlContent_btnLogin")
	private WebElement submitLogin;
	
	@FindBy(id = "ctl00_Header1_ctlHeaderActionBar_ctlMemberNavigation_divLogout")
	private WebElement logoutBtn;
	
	
	public void goToLogin(){
		waitUntilAppears(loginBtn);		
		loginBtn.click();
	}
	
	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){

		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void submit(){
		submitLogin.click();
		}
	
	public boolean checkLogin(){

		waitUntilAppears(By.id("ctl00_Header1_ctlHeaderActionBar_ctlMemberNavigation_divLogout"));
		return isElementPresent(By.id("ctl00_Header1_ctlHeaderActionBar_ctlMemberNavigation_divLogout"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("ctl00_phlContent_txtUsername"));
	}
	
	public void logout(){
		logoutBtn.click();
	}
}
