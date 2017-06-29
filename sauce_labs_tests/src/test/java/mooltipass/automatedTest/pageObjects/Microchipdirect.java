package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Microchipdirect extends AbstractPage{

	public Microchipdirect(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}
	@FindBy(xpath = "//input[@id='_ctl0_cphRightMaster_txtEmail']")
	private WebElement email;

	@FindBy(xpath = "//input[@id='_ctl0_cphRightMaster_txtPassword']")
	private WebElement password;

	@FindBy(xpath = "//a[@id='_ctl0_cphRightMaster_lnkLogin']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//a[contains(@href,'loginuser.aspx?mode=login')]")
	private WebElement loginBtn;
	
	@FindBy(xpath = "//a[contains(@href,'loginuser.aspx?mode=logout')]")
	private WebElement logoutBtn;
	

	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}
	
	public void enterPassword(String value){

		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void goToLogin(){

		waitUntilAppears(loginBtn);	
		clickWithAction(loginBtn);
	}

	public void submit(){
		waitUntilAppears(submitLogin);
		submitLogin.click();
	}

	public void logout(){
		waitUntilAppears(logoutBtn);
		clickWithAction(logoutBtn);
	}
	public boolean checkLogin(){

		waitUntilAppears(By.xpath("//a[contains(@href,'loginuser.aspx?mode=logout')]"));
		return isElementPresent(By.xpath("//a[contains(@href,'loginuser.aspx?mode=logout')]"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("_ctl0_cphRightMaster_txtEmail"));
	}
}

