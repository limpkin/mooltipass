package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class ProtonMail  extends AbstractPage{
	
	public ProtonMail (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(xpath = "//a[@title='Log In']")
	private WebElement loginBtn;
	
	@FindBy(id = "username")
	private WebElement email;
	
	@FindBy(id = "password")
	private WebElement password;
	
	@FindBy(id = "login_btn")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//a[@class='pm_button primary text-center navigationUser-logout']")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//i[@class='fa fa-user']")
	private WebElement dashBoard;
	
	public void goToLogin(){
		waitUntilAppears(loginBtn);
		loginBtn.click();
	}
	
	public void enterEmail(String value){
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		waitUntilAppears(password);
		password.sendKeys(value);
	}

	
	public void submit(){

		submitLogin.click();
	}

	public void logout(){	

		waitUntilAppears(logoutBtn);
		logoutBtn.click();
	}
	public void goTodDashboard()
	{
		dashBoard.click();
		
		
	}
	public boolean checkLogin(){
		waitUntilAppears(By.xpath("//i[@class='fa fa-user']"));
		return isElementPresent(By.xpath("//i[@class='fa fa-user']"));
	}
	public boolean checkAtLoginPage(){
		waitUntilAppears(By.id("username"));
		return isElementPresent(By.id("username"));
	}
}
